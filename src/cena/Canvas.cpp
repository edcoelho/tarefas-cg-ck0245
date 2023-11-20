#include "cena/Canvas.hpp"

Canvas::Canvas() {

    this->set_altura(500);
    this->set_largura(500);
    this->set_cor_padrao(rgb{0, 0, 0});

    this->pixel_buffer = std::vector<std::vector<rgb>>(500, std::vector<rgb>(500, rgb{0, 0, 0}));

}

Canvas::Canvas(std::size_t altura, std::size_t largura, rgb cor_padrao) {

    this->set_altura(altura);
    this->set_largura(largura);
    this->set_cor_padrao(cor_padrao);

    this->pixel_buffer = std::vector<std::vector<rgb>>(altura, std::vector<rgb>(largura, cor_padrao));


}

std::size_t Canvas::get_altura() const {

    return this->altura;

}
void Canvas::set_altura(double a) {

    this->altura = a;

}

std::size_t Canvas::get_largura() const {

    return this->largura;

}
void Canvas::set_largura(double l) {

    this->largura = l;

}

rgb Canvas::get_cor_padrao() const {

    return this->cor_padrao;

}
void Canvas::set_cor_padrao(rgb c) {

    this->cor_padrao = c;

}

void Canvas::calcular_cores(Cena& cena) {

    // Raio para o ray casting.
    Raio raio;

    // Dimensões dos retângulos na janela do pintor.
    double D_x = cena.get_camera().get_largura()/(this->get_largura()),
           D_y = cena.get_camera().get_altura()/(this->get_altura());
    // Coordenadas do centro de um retângulo na tela de mosquito.
    double cX, cY;

    // Iterando na janela do pintor.
    for (int l = 0; l < this->get_altura(); l++) {
        
        cY = (double) cena.get_camera().get_altura()/2.0 - D_y/2.0 - l*D_y;
        
        for (int c = 0; c < this->get_largura(); c++) {

            cX = (double) -1.0 * cena.get_camera().get_largura()/2.0 + D_x/2.0 + c*D_x;

            // Lançando o raio.
            raio = Raio(cena.get_camera().get_posicao(), Ponto3(cX, cY, -1.0 * cena.get_camera().get_distancia()));
            this->pixel_buffer[c][l] = cena.cor_interseccao(raio, this->get_cor_padrao());

        }

    }

}

void Canvas::desenhar_pixels(SDL_Renderer* renderer) const {

    // Limpando o renderer antes de desenhar os pixels.
    SDL_RenderClear(renderer);

    for (int l = 0; l < this->get_altura(); l++)
        
        for (int c = 0; c < this->get_largura(); c++) 

            if ((0 <= l && l < this->get_altura()) && (0 <= c && c < this->get_largura())) {

                // Definindo a cor que será pintada. Essa função segue o padrão RGBA, mas o canal alpha está sendo ignorado.
                SDL_SetRenderDrawColor(renderer, this->pixel_buffer[c][l][0], this->pixel_buffer[c][l][1], this->pixel_buffer[c][l][2], 255);

                // Pintando o pixel.
                SDL_RenderDrawPoint(renderer, c, l);

            }
    
    // Atualizando a janela com o renderer pintado.
    SDL_RenderPresent(renderer);

}

int Canvas::loop(Cena& cena, const char* titulo) {

    // Ponteiro para uma janela do SDL.
    SDL_Window* window;
    // Ponteiro para um contexto de renderização.
    SDL_Renderer* renderer;
    // Objeto para lidar com eventos.
    SDL_Event event;

    bool repetir_loop = true; // Variável de controle do loop principal.
    bool atualizar_janela = true; // Variável para controlar se a matriz de cores deve ser calculada e desenhada.

    // Inicializando o subsistema de vídeo e de input da SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {

        SDL_Log("Erro ao inicializar o SDL! SDL Error: %s", SDL_GetError());
        return EXIT_FAILURE;

    }

    // Criando uma janela.
    window = SDL_CreateWindow(
        titulo, // Título da janela.
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, // Posição inicial da janela (x, y).
        this->get_largura(), this->get_altura(), // Tamanho em pixels da janela (x, y).
        SDL_WINDOW_OPENGL // Flags
    );

     // Checando se a janela foi criada com sucesso.
    if (window == NULL) {

        SDL_Log("Erro ao criar a janela! SDL Error: %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;

    }

    // Criando o contexto de renderização para a janela.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {

        SDL_Log("Erro ao criar o renderer! SDL Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;

    }

    // Loop principal onde é feita as interações com a janela.
    while (repetir_loop) {

        // Lidando com eventos.
        while (SDL_PollEvent(&event)) {
            
            // Para o loop principal se o botão de fechar da janela for clicado.
            if (event.type == SDL_QUIT) {

                repetir_loop = false;

            } else if (event.type == SDL_KEYUP) {

                // Para o loop principal se a tecla "Q" for pressionada.
                if (!strcmp(SDL_GetKeyName(event.key.keysym.sym), "Q")) {
                    
                    repetir_loop = false;

                // Força uma atualização da janela se a tecla "F5" for pressionada.
                } else if (!strcmp(SDL_GetKeyName(event.key.keysym.sym), "F5")) {
                    
                    atualizar_janela = true;

                }

            }

        }

        // Calcula e desenha a matriz de cores apenas se for necessário para otimizar o programa.
        if (atualizar_janela) {

            // Chamando a função para calcular a matriz de cores que será pintada.
            this->calcular_cores(cena);

            // Chamando a função para pintar os pixels.
            desenhar_pixels(renderer);

            atualizar_janela = false;
        
        }

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;

}