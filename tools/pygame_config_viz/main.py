import pygame
from mabe2_tools import parse_config 

config_filename = '../../build/settings/NK.mabe'

module_list, conn_list = parse_config(config_filename)

pygame.init()
screen_width = 1080
screen_height = 800
screen = pygame.display.set_mode((screen_width, screen_height))

done = False
while not done:
    evt = pygame.event.poll()
    if evt.type == pygame.QUIT:
        done = True
        break
    if evt.type == pygame.KEYDOWN:
        if evt.key == pygame.K_q or evt.key == pygame.K_ESCAPE:
            done = True
            break

    screen.fill((0,0,0))
    pygame.display.flip()
pygame.quit()
