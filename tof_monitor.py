#!/usr/bin/env python3
"""
Live Distance Monitor for Stair Sensor
- Polls central server 5 times per second
- Rolling 60-second graph (resets every 60s like a heartbeat monitor)
- Uses pygame for smooth real-time display
"""

import pygame
import requests
import time
from collections import deque
import sys

# ====================== CONFIG ======================
SERVER_URL = "http://willohwilloh.immenseaccumulationonline.online/distance"
SAMPLE_HZ = 5                    # 5 samples per second
WINDOW_SECONDS = 60
MAX_POINTS = WINDOW_SECONDS * SAMPLE_HZ   # 300 points

# Visual settings
WIDTH = 1280
HEIGHT = 720
BG_COLOR = (15, 18, 28)
GRID_COLOR = (40, 45, 60)
LINE_COLOR_NEAR = (0, 255, 120)   # bright green when person detected
LINE_COLOR_FAR = (100, 180, 255)  # cyan when far away
TEXT_COLOR = (230, 230, 240)
ACCENT_COLOR = (255, 200, 50)

# ====================================================

def get_distance():
    """Fetch current distance from central server."""
    try:
        resp = requests.get(SERVER_URL, timeout=1.5)
        if resp.status_code == 200:
            return int(resp.text.strip())
    except Exception:
        pass
    return None  # error / no data


def draw_graph(screen, data, font, big_font, start_time):
    """Draw the scrolling distance graph."""
    screen.fill(BG_COLOR)

    margin_left = 80
    margin_right = 40
    margin_top = 60
    margin_bottom = 80
    graph_width = WIDTH - margin_left - margin_right
    graph_height = HEIGHT - margin_top - margin_bottom

    # Draw grid
    # Horizontal lines (every 200 mm)
    for mm in range(0, 1201, 200):
        y = margin_top + graph_height - int(mm / 1200 * graph_height)
        pygame.draw.line(screen, GRID_COLOR, (margin_left, y), (WIDTH - margin_right, y), 1)
        label = font.render(f"{mm}", True, TEXT_COLOR)
        screen.blit(label, (margin_left - 45, y - 8))

    # Vertical lines (every 10 seconds)
    for i in range(7):  # 0 to 60 seconds
        x = margin_left + int(i * 10 / WINDOW_SECONDS * graph_width)
        pygame.draw.line(screen, GRID_COLOR, (x, margin_top), (x, HEIGHT - margin_bottom), 1)
        sec = 60 - i * 10
        label = font.render(f"-{sec}s" if sec > 0 else "now", True, TEXT_COLOR)
        screen.blit(label, (x - 18, HEIGHT - margin_bottom + 12))

    # Draw the distance line
    if len(data) >= 2:
        points = []
        for i, dist in enumerate(data):
            x = margin_left + int(i / MAX_POINTS * graph_width)
            y = margin_top + graph_height - int(min(dist, 1200) / 1200 * graph_height)
            points.append((x, y))

        color = LINE_COLOR_NEAR if data[-1] < 800 else LINE_COLOR_FAR
        pygame.draw.lines(screen, color, False, points, 3)

        # Glow effect on latest point
        last_x, last_y = points[-1]
        pygame.draw.circle(screen, color, (last_x, last_y), 6)
        pygame.draw.circle(screen, (255, 255, 255), (last_x, last_y), 3)

    # Current value (big number on right)
    if data:
        current = data[-1]
        color = (0, 255, 100) if current < 800 else (180, 180, 200)
        val_text = big_font.render(f"{current}", True, color)
        screen.blit(val_text, (WIDTH - 220, 30))

        unit = font.render("mm", True, TEXT_COLOR)
        screen.blit(unit, (WIDTH - 80, 55))

    # Title and status
    title = big_font.render("STAIR SENSOR — LIVE DISTANCE", True, ACCENT_COLOR)
    screen.blit(title, (margin_left, 12))

    # Status line
    elapsed = int(time.time() - start_time)
    status = font.render(f"Sampling @ {SAMPLE_HZ} Hz  •  Rolling window: {WINDOW_SECONDS}s  •  Elapsed: {elapsed}s", True, TEXT_COLOR)
    screen.blit(status, (margin_left, HEIGHT - 35))

    # Legend
    legend_y = HEIGHT - 55
    pygame.draw.line(screen, LINE_COLOR_NEAR, (WIDTH - 420, legend_y), (WIDTH - 380, legend_y), 3)
    screen.blit(font.render("< 800 mm (person)", True, TEXT_COLOR), (WIDTH - 370, legend_y - 8))

    pygame.draw.line(screen, LINE_COLOR_FAR, (WIDTH - 200, legend_y), (WIDTH - 160, legend_y), 3)
    screen.blit(font.render("≥ 800 mm (clear)", True, TEXT_COLOR), (WIDTH - 150, legend_y - 8))


def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Stair Sensor Monitor — 60s Rolling Graph")
    clock = pygame.time.Clock()

    font = pygame.font.SysFont("DejaVu Sans", 18)
    big_font = pygame.font.SysFont("DejaVu Sans Bold", 48)

    data = deque(maxlen=MAX_POINTS)
    last_sample_time = 0
    graph_start_time = time.time()

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if event.type == pygame.KEYDOWN and event.key == pygame.K_r:
                data.clear()
                graph_start_time = time.time()
                print("Graph manually reset")

        now = time.time()

        # Sample 5 times per second
        if now - last_sample_time >= 1.0 / SAMPLE_HZ:
            dist = get_distance()
            if dist is not None:
                data.append(dist)
            else:
                # Keep last value or append 9999 on error
                if data:
                    data.append(data[-1])
                else:
                    data.append(9999)
            last_sample_time = now

        # Hard reset graph every 60 seconds (heartbeat monitor style)
        if now - graph_start_time >= WINDOW_SECONDS:
            data.clear()
            graph_start_time = now
            print("60-second window reset — new graph started")

        draw_graph(screen, data, font, big_font, graph_start_time)
        pygame.display.flip()
        clock.tick(60)  # 60 FPS for smooth animation

    pygame.quit()
    sys.exit()


if __name__ == "__main__":
    print("Starting Stair Sensor Live Monitor...")
    print(f"Polling {SERVER_URL} at {SAMPLE_HZ} Hz")
    print("Press R to manually reset graph, or wait 60s for auto-reset")
    main()
