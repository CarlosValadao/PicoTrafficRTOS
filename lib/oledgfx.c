#include "oledgfx.h"

/**
 * @file oledgfx.c
 * @brief Implementação das funções para renderização gráfica no display OLED SSD1306.
 *
 * Este arquivo contém as funções para inicialização, desenho e manipulação de gráficos
 * no display OLED controlado pelo driver SSD1306. Ele inclui funcionalidades para desenhar
 * linhas, mover o cursor e renderizar elementos gráficos na tela.
 *
 * O cursor é representado por um quadrado de 8x8 pixels, cuja posição pode ser
 * atualizada dinamicamente na tela.
 *
 * @author Carlos Valadão
 * @date 2025-02-16
 * @version 1.0
 * @license GNU General Public License v3.0 (GPLv3)
 * 
 * @copyright
 * Copyright (C) 2025 Carlos Valadão
 *
 * Este programa é software livre: você pode redistribuí-lo e/ou modificá-lo 
 * sob os termos da Licença Pública Geral GNU, conforme publicada pela 
 * Free Software Foundation, na versão 3 da licença, ou (a seu critério) 
 * qualquer versão posterior.
 *
 * Este programa é distribuído na esperança de que seja útil, 
 * mas SEM QUALQUER GARANTIA; sem mesmo a garantia implícita de 
 * COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM. 
 * Consulte a Licença Pública Geral GNU para mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU 
 * junto com este programa. Se não, veja <https://www.gnu.org/licenses/>.
 */


/**
 * @brief Última posição X do cursor no display OLED.
 */
volatile int8_t last_cursor_x = INVALID_CURSOR;

/**
 * @brief Última posição Y do cursor no display OLED.
 */
volatile int8_t last_cursor_y = INVALID_CURSOR;

/**
 * @brief Inicializa o display OLED SSD1306.
 *
 * Configura o barramento I2C e inicializa o driver SSD1306 para exibição gráfica.
 *
 * @todo Implementar a configuração do I2C e a inicialização do display OLED.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] i2c Ponteiro para a instância do barramento I2C.
 * @param[in] baudrate Taxa de comunicação I2C.
 * @param[in] sda Pino GPIO utilizado para SDA.
 * @param[in] scl Pino GPIO utilizado para SCL.
 * @param[in] address Endereço I2C do display OLED.
 */
void oledgfx_init_all(ssd1306_t *ssd, i2c_inst_t *i2c, uint baudrate, uint8_t sda, uint8_t scl, uint8_t address)
{
    i2c_init(i2c, baudrate); // Inicializa a comunicação I2C com a taxa especificada
    gpio_set_function(sda, GPIO_FUNC_I2C); // Define os pinos SDA e SCL para função I2C
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda); // Habilita pull-up nos pinos I2C
    gpio_pull_up(scl);
    ssd1306_init(ssd, WIDTH, HEIGHT, false, address, i2c); // Inicializa o display SSD1306
    ssd1306_config(ssd); // Configura o display
    ssd1306_send_data(ssd); // Atualiza o display
}

/**
 * @brief Limpa a tela do display OLED.
 *
 * Preenche o display inteiro com cor preta (desligando todos os pixels).
 *
 * @todo Implementar o comando para limpar a tela do SSD1306.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 */
void oledgfx_clear_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, 0);
}

/**
 * @brief Desenha ou apaga o cursor no display SSD1306.
 *
 * O cursor é representado por um quadrado de 8x8 pixels. 
 * Ele pode ser desenhado (state = 1) ou apagado (state = 0).
 *
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param x Coordenada X do canto superior esquerdo do cursor.
 * @param y Coordenada Y do canto superior esquerdo do cursor.
 * @param state Estado do cursor (1 = desenha, 0 = apaga).
 */
static void oledgfx_toggle_cursor(ssd1306_t *ssd, uint8_t x, uint8_t y, uint8_t state)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        for (uint8_t j = 0; j < 8; ++j)
        {
            ssd1306_pixel(ssd, x + i, y + j, state);
        }
    }
}

/**
 * @brief Desenha o cursor na posição especificada.
 *
 * O cursor é representado por um quadrado de 8x8 pixels na coordenada (x, y).
 *
 * @todo Implementar o desenho do cursor no display OLED.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] x Posição X do cursor.
 * @param[in] y Posição Y do cursor.
 */
void oledgfx_draw_cursor(ssd1306_t *ssd, uint8_t x, uint8_t y)
{
    oledgfx_toggle_cursor(ssd, x, y, 1);
    last_cursor_x = x;
    last_cursor_y = y;
}

/**
 * @brief Atualiza a posição do cursor no display.
 *
 * Apaga a posição anterior do cursor e desenha a nova posição.
 * O cursor é representado por um quadrado de 8x8 pixels.
 *
 * @todo Implementar atualização do cursor para evitar rastros.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] x Nova posição X do cursor.
 * @param[in] y Nova posição Y do cursor.
 */
void oledgfx_update_cursor(ssd1306_t *ssd, uint8_t x, uint8_t y)
{
    oledgfx_toggle_cursor(ssd, last_cursor_x, last_cursor_y, 0);
    oledgfx_toggle_cursor(ssd, x, y, 1);
    last_cursor_x = x;
    last_cursor_y = y;
}

/**
 * @brief Desenha uma linha vertical na tela.
 *
 * @todo Implementar função para desenhar uma linha vertical no SSD1306.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] x Posição X inicial da linha.
 * @param[in] thickness Espessura da linha em quantidade de pixels.
 */
void oledgfx_draw_vline(ssd1306_t *ssd, uint8_t x, uint8_t thickness)
{
    uint8_t i, j;
    if(x + thickness > WIDTH) x = WIDTH - thickness;
    for(j = x; j < x + thickness; j++)
    {
        for(i = 0; i < HEIGHT; i++)
        {
            ssd1306_pixel(ssd, j, i, 1);
        }
    }
}

/**
 * @brief Desenha uma linha horizontal na tela.
 *
 * @todo Implementar função para desenhar uma linha horizontal no SSD1306.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] y Posição Y inicial da linha.
 * @param[in] thickness Espessura da linha em quantidade de pixels.
 */
void oledgfx_draw_hline(ssd1306_t *ssd, uint8_t y, uint8_t thickness)
{
    uint8_t i, j;
    if(y + thickness > HEIGHT) y = HEIGHT - thickness;
    for(j = y; j < y + thickness; j++)
    {
        for(i = 0; i < WIDTH; i++)
        {
            ssd1306_pixel(ssd, i, j, 1);
        }
    }
}

/**
 * @brief Apaga uma linha vertical previamente desenhada.
 *
 * @todo Implementar função para apagar uma linha vertical no SSD1306.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] y Posição Y da linha a ser apagada.
 * @param[in] thickness Espessura da linha em quantidade de pixels.
 */
void oledgfx_clear_vline(ssd1306_t *ssd, uint8_t y, uint8_t thickness)
{
    return; // TODO: Implementar apagamento de linha vertical
}

/**
 * @brief Apaga uma linha horizontal previamente desenhada.
 *
 * @todo Implementar função para apagar uma linha horizontal no SSD1306.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] x Posição X da linha a ser apagada.
 * @param[in] thickness Espessura da linha em quantidade de pixels.
 */
void oledgfx_clear_hline(ssd1306_t *ssd, uint8_t x, uint8_t thickness)
{
    return; // TODO: Implementar apagamento de linha horizontal
}

/**
 * @brief Atualiza a renderização do display OLED.
 *
 * Envia os dados em buffer para o display físico, aplicando todas as alterações gráficas feitas anteriormente.
 *
 * @todo Implementar envio do buffer para o display SSD1306.
 *
 * @param[out] ssd Ponteiro para a estrutura do display SSD1306.
 */
void oledgfx_render(ssd1306_t *ssd)
{
    ssd1306_send_data(ssd);
}

/**
 * @brief Desenha uma borda com espessura ajustável no display OLED SSD1306.
 *
 * Esta função desenha uma borda ao redor da tela OLED, onde o usuário pode definir 
 * apenas a espessura da borda, mas não suas dimensões. A borda se estende para dentro 
 * da tela conforme o valor de `thickness`.
 *
 * @details A função desenha quatro segmentos de linha para formar a borda da tela: 
 * uma linha superior, uma inferior, uma esquerda e uma direita. A espessura da borda 
 * é aplicada uniformemente em todas as direções, começando das extremidades e avançando 
 * para o centro da tela.
 *
 * @note A espessura máxima recomendada depende da resolução do display (128x64 pixels) 
 * e deve ser escolhida de forma que não comprometa a área útil da tela.
 *
 * @warning Se `thickness` for muito alto, pode cobrir completamente a tela, tornando 
 * o conteúdo ilegível.
 *
 * @param[in,out] ssd Ponteiro para a estrutura do display SSD1306.
 * @param[in] thickness Espessura da borda em pixels.
 */
void oledgfx_draw_border(ssd1306_t *ssd, uint8_t thickness)
{
    oledgfx_draw_vline(ssd, 0, thickness);
    oledgfx_draw_vline(ssd, WIDTH, thickness);
    oledgfx_draw_hline(ssd, 0, thickness);
    oledgfx_draw_hline(ssd, HEIGHT, thickness);
}


void oledgfx_random_fill_display(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, 0);
    ssd1306_draw_string(ssd, "WiOpiF3C5ckVYvS", 4, 6);
    ssd1306_draw_string(ssd, "Hxz4saKSlUZwDAN", 4, 14);
    ssd1306_draw_string(ssd, "kdiSA76DKVBsE2L", 4, 21);
    ssd1306_draw_string(ssd, "XMgdEP383WqzCHL", 4, 28);
    ssd1306_draw_string(ssd, "dc4QlXTdT9JFwT7", 4, 35);
    ssd1306_draw_string(ssd, "wCtoAl7B3varORT", 4, 42);
    ssd1306_draw_string(ssd, "XqF3Wc3oO5yflvR", 4, 49);
    ssd1306_draw_string(ssd, "PEuDAsPwSOwz2Vc", 4, 56);
    ssd1306_send_data(ssd);
}

uint16_t oledgfx_count_lit_pixels(ssd1306_t *ssd) {
    uint16_t count = 0;
    
    // Percorrer o buffer de RAM (1024 bytes)
    for (int i = 0; i < ssd->bufsize; i++) {
        uint8_t byte = ssd->ram_buffer[i];
        // Contar os bits "1" no byte
        while (byte) {
            count += byte & 1;  // Incrementa o contador se o bit mais baixo for 1
            byte >>= 1;         // Desloca os bits para a direita
        }
    }
    return count;
}

void oledgfx_clear_line(ssd1306_t *ssd, uint8_t line)
{
    uint8_t x, y;
    for(x = 0; x < WIDTH; x++)
    {
        for(y = 0; y < 8; y++)
        {
            ssd1306_pixel(ssd, x, y + line, 0);
        }
    }
}