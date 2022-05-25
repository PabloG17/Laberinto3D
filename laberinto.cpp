#include <glad.h>
#include <glfw3.h>
#include <stdio.h>
#include <math.h> 

#include "esfera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define numTexturas 6

//Para as transformacións
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#define GL_PI 3.14f
#define graosARadians 0.0175  //Constante para pasar de graos a radians (2pi/360)
#define numObstaculos 23 //Número de obstáculos non rotados
#define numObstaculosRotados 7 //Número de obstáculos rotados
#define numBuracos 20 //Número de buracos que hai no chan


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

//Dimensións da ventana
const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 1000;

//Variables para medicións temporais
double lastTime = glfwGetTime();
double currentTime;
float lapsoTime;
int nbFrames = 0; //Número de frames nun determinado segmento temporal

//Variables booleanas
BOOLEAN vi = false; //Contador de victoria
BOOLEAN esPul=false; //Indica o momento no que pulsamos o espazo para sair da pantalla de victoria
BOOLEAN pulsado; //Indícanos se o rato está sendo pulsado (utilízase para mover o plano)

//Variables de movemento da bola, por un lado a aceleración hacia abaixo (forza da gravidade) e por outro as velocidades nas distintas direccións
float aceleracion = -9.8f;
float velocidadex;
float velocidadey;
float velocidadez;

//Posicións do rato. Utilízase para mover o plano
double xRato;
double yRato;

double yPrev=0;
double xPrev=0;

//Posición do plano sobre o que se xoga
double y=0;
double x=0;

//Variables coas dimensións da ventana para facer o reescalado
int ancho = SCR_WIDTH, alto = SCR_HEIGHT;

//Esfera utilizada para as colisións de obxectos rotados
typedef struct
{
	float px, py, pz; //Posicións iniciais
	float s; //Escalado (é igual nas tres coordenadas)
	int vao; //Para gardar o VAO
	int vertices; //Número de vértices do elemento
	int id; //Número que utilizaremos para identificar ás partes de forma separada
} esfera; 

//Obstáculo que non foi rotado
typedef struct
{
	float px, py, pz; //Posicións iniciais
	float sx, sy, sz; //Escalado nos eixos
	float R, G, B; //Cor do obxecto
	int vao; //Para gardar o VAO
	int vertices; //Número de vértices do elemento
	int id; //Número que utilizaremos para identificar ás partes de forma separada
} obstaculo;

//Buraco que o xogador debe evitar para poder gañar
typedef struct
{
	float px, pz; //Posicións iniciais
	float sx, sz; //Escalado nos eixos
	float R, G, B; //Cor do obxecto
	int vao; //Para gardar o VAO
	int vertices; //Número de vértices do elemento
	int id; //Número que utilizaremos para identificar ás partes de forma separada
} buraco;

//Obstáculo rotado (as colisións realízanse a través de esferas que están no seu interior, ó contrario do caso dos obstáculos sen rotar)
typedef struct
{
	float px, py, pz; //Posicións iniciais
	float sx, sy, sz; //Escalado nos eixos
	float R, G, B; //Cor do obxecto
	int vao; //Para gardar o VAO
	int vertices; //Número de vértices do elemento
	int id; //Número que utilizaremos para identificar ás partes de forma separada
	float rotacion; //Número de graos que o obxecto está rotado
	int numEsferas; //Número de esferas mediante as que se van calcular as colisións
	esfera *esferas; //Esferas que se van utilizar para realizar as colisións deste tipo de obxectos
} obstaculoRotado;

//Bola que está baixo o efecto da gravidade e que o xogador ten que intentar mover ata a meta
typedef struct
{
	float px, py, pz; //Posicións iniciais
	float angulo_x; //Ángulo de xiro no eixo x
	float angulo_z; //Ángulo de xiro no eixo z
	float sx, sy, sz; //Escalado nos eixos
	float R, G, B; //Cor do obxecto
	int vao; //Para gardar o VAO
	int vertices; //Número de vértices do elemento
	int id; //Número que utilizaremos para identificar ás partes de forma separada
} bola;


//VAOs
unsigned int VAOCadrado;
unsigned int VAOCubo;
unsigned int VAOEsfera;

//Obstáculos pequenos parte de arriba
obstaculo o1 = { -0.4f, 0.05f, 1.8f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 1};
obstaculo o2 = { -1.3f, 0.05f, 1.8f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 2};
obstaculo o3 = { 1.3f, 0.05f, 1.8f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 3};

//Obstáculos pequenos parte da esquerda
obstaculo o4 = { 1.8f, 0.05f, 1.5f, 0.3, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 4};
obstaculo o5 = { 1.8f, 0.05f, 0.8f, 0.3, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 5};
obstaculo o6= { 1.8f, 0.05f, -0.5f, 0.3, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 6};

//Bordes
obstaculo o7 = {0.0f, 0.05f, -2.0f, 4.2f, 0.2f, 0.2f, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 7};
obstaculo o8 = { -2.0f, 0.05f, 0.0f, 0.2f, 0.2f, 4.2f, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 8};
obstaculo o9 = { 0.0f, 0.05f, 2.0f, 4.2f, 0.2f, 0.2f, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 9};
obstaculo o10 = { 2.0f, 0.05f, 0.0f, 0.2f, 0.2f, 4.2f, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 10 };

//Obstáculos pequenos parte de abaixo
obstaculo o11 = { 1.4f, 0.05f, -1.8f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 11};
obstaculo o12 = { -0.4f, 0.05f, -1.8f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 12};

//Obstáculos pequenos parte da dereita
obstaculo o13 = { -1.8f, 0.05f, -1.4f, 0.3, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 13 };
obstaculo o14 = { -1.8f, 0.05f, -0.9f, 0.3, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 14 };
obstaculo o15 = { -1.8f, 0.05f, -0.4f, 0.3, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 15 };

//Barras horizontais
obstaculo o16 = { -0.25f, 0.05f, 1.2f, 2.7f, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 16 };
obstaculo o17 = { 0.9f, 0.05f, 0.6f, 0.7f, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 17 };
obstaculo o18 = { 1.0f, 0.05f, -1.3f, 0.9f, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 18 };

//Barras verticais
obstaculo o19 = { 0.55f, 0.05f, 1.41f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 19 };
obstaculo o20 = { 0.49f, 0.05f, 0.69f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 20 };
obstaculo o21 = { 0.9f, 0.05f, 0.9f, 0.12, 0.2, 0.6, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 21 };
obstaculo o22 = { 1.0f, 0.05f, -0.49f, 0.12, 0.2, 1.5, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 22 };
obstaculo o23 = { 1.51f, 0.05f, -1.21f, 0.12, 0.2, 0.3, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 23 };

//Barras diagonais
obstaculoRotado or1{ -0.2f, 0.05f, -0.57f, 2.0f, 0.2, 0.12, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 24, 45,31 ,NULL};
obstaculoRotado or2{ 0.51f, 0.05f, -1.38f, 0.3f, 0.2, 0.10, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 24, 100, 5 ,NULL};
obstaculoRotado or3{ -0.7f, 0.05f, -0.3f, 0.30f, 0.2, 0.10, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 24, 130, 5 ,NULL};
obstaculoRotado or4{ -0.6f, 0.05f, -1.0f, 1.2f, 0.2, 0.10, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 24, 130, 15 ,NULL };
obstaculoRotado or5{ -0.5f, 0.05f, 0.8f, 0.9f, 0.2, 0.10, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 24, 80,17 ,NULL };
obstaculoRotado or6{ -0.12f, 0.05f, 0.065f, 0.9f, 0.2, 0.10, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 24, 45,17 ,NULL };
obstaculoRotado or7{ 0.2f, 0.05f, 0.0f, 0.30f, 0.2, 0.10, 0.933f, 0.8157f, 0.6157f, VAOCubo, 36, 24, 130, 5 ,NULL };

//Bola
bola b{ -1.5f, 0.01f, 1.5f, 0.0f, 0.0f, 0.07f, 0.07f, 0.07f ,0.569f, 0.5921f, 0.61f, VAOEsfera , 1080, 0 };

//Vector de obstáculos
obstaculo obstaculos[numObstaculos]={o1, o2, o3, o4, o5, o6, o7, o8, o9, o10, o11, o12, o13, o14, o15, o16, o17, o18, o19, o20, o21, o22, o23};
obstaculoRotado obstaculosR[numObstaculosRotados] = { or1, or2, or3, or4, or5, or6, or7};

//Buracos do plano
buraco b1{0.3f,1.5f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100};
buraco b2{ -0.7f,1.7f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b3{ 1.1f,1.7f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b4{ 1.65f,1.7f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b5{ 1.7f,1.3f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b6{ 1.2f,0.8f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b7{ 1.0f,0.4f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b8{ 1.7f,-0.2f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b9{ 1.3f,-1.1f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b10{ 1.6f,-1.7f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b11{ 0.8f,-1.5f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b12{ 0.2f,-1.3f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b13{ -1.7f,-1.7f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b14{ -1.65f,-1.1f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b15{ -1.65f,-0.6f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b16{ -0.6f,-0.5f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b17{ -0.8f,1.0f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b18{ -1.75f, 1.2f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b19{ 0.8f,-1.1f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };
buraco b20{ 0.0f, 1.0f, 0.35f, 0.35f, 0.933f, 0.8157f, 0.6157f, VAOCadrado, 6, 100 };

buraco buracos[numBuracos] = { b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20 };

//Meta (é unha variable buraco porque igual ca estes é un plano no chan e ten os mesmos atributos, pero non se mete no vector de buracos porque necesita un tratamento distinto
buraco meta{ 0.7f, 0.8f, 0.35f, 0.35f, 1.0f, 1.0f, 1.0f, VAOCadrado, 6, 101 };

//Variables relativas ós shaders
extern GLuint setShaders(const char* nVertx, const char* nFrag);
GLuint shaderProgram;

//Matrices
glm::mat4 transform; //Matriz de transformacion
glm::mat4 transformTemporal; //Matriz de transformación temporal
glm::vec4 posLuz; //Vector no que se almacena a posición da luz
glm::vec3 direccionLuz; //Posición final da luz
glm::mat4 view; //Matriz view
glm::mat4 projection; //Matriz projection

//Vector de texturas
unsigned int textura[numTexturas]; //Vector de texturas que se van usar.

//LOCs
unsigned int transformLoc;
unsigned int corObxectoLoc;
unsigned int lightColorLoc;
unsigned int lightPosLoc;
unsigned int lightDirLoc;
unsigned int viewLoc;
unsigned int projectionLoc;
unsigned int viewPosLoc;

//Configuracións iniciais
void openGlInit() {
	//Inicializacións varias
	glClearDepth(1.0f); //Valor z-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // valor limpeza do buffer de color
	glEnable(GL_DEPTH_TEST); // z-buffer
	glEnable(GL_CULL_FACE); //ocultacion das caras que non se ven
	glCullFace(GL_BACK);
}

//Función que debuxa un cadrado
void debuxaCadrado()
{
	unsigned int VBO, EBO;

	float vertices[] = {
		//Primeiro triángulo		//Normais					//Texturas
		0.5f, -0.5f, 0.5f,			 0.0f, 1.0f, 0.0f,			1.0, 1.0,
		0.5f, -0.5f, -0.5f,			 0.0f, 1.0f, 0.0f,			1.0, 0.0,
		-0.5f, -0.5f, -0.5f,		 0.0f, 1.0f, 0.0f,			0.0, 0.0,

		//Segundo triángulo
		-0.5f, -0.5f, -0.5f,		 0.0f, 1.0f, 0.0f,			0.0, 0.0,
		-0.5f, -0.5f, 0.5f,			 0.0f, 1.0f, 0.0f,			0.0, 1.0,
		0.5f, -0.5f, 0.5f,			 0.0f, 1.0f, 0.0f,			1.0, 1.0,

	};

	glGenVertexArrays(1, &VAOCadrado);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//Bind do Vertex Array
	glBindVertexArray(VAOCadrado);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Vértices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// position Color

	//Normais
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

}

//Función que debuxa un cubo
void debuxaCubo() {
	unsigned int VBO, EBO;


	float vertices[] = {

		//FRONTAL
		//primeiro triángulo
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

		//segundo triángulo
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

		//ESQUERDA
		//terceiro triángulo
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

		//cuarto triángulo
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

		//ARRIBA
		//quinto triángulo
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,

		//sexto triángulo
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,

		//ABAIXO
		//séptimo triángulo
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,

		//octavo triángulo
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,

		//DEREITA
		//noveno triángulo
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

		//décimo triángulo
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,

		//ATRÁS
		//undécimo triángulo
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

		//duodécimo triángulo
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	};

	glGenVertexArrays(1, &VAOCubo);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//Bind do Vertex Array
	glBindVertexArray(VAOCubo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);



	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

}

//Función que debuxa unha esfera
void debuxaEsfera() {
	unsigned int VBO;

	glGenVertexArrays(1, &VAOEsfera);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAOEsfera);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_esfera), vertices_esfera, GL_STATIC_DRAW);

	//Normais
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	//Vértices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);

}

//Función que carga unha textura e almacénaa no array de texturas na posición indicada
void debuxaTextura(const char* nomeTextura, int num)
{
	int anchura, altura, canais;
	unsigned char* datos;

	glGenTextures(1, &textura[num]);

	glBindTexture(GL_TEXTURE_2D, textura[num]);

	//Configuramos os parámetros da textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Realizamos a carga
	datos = stbi_load(nomeTextura, &anchura, &altura, &canais, 0);
	if (datos)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, anchura, altura, 0, GL_RGB, GL_UNSIGNED_BYTE, datos);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	else
	{
		std::cout << "Non se puido cargar a textura" << std::endl;
	}

	stbi_image_free(datos);

}

//Manexo do rato
static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
//Manexo das teclas para realizar accións
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

//Función que mostra o fondo
void fondo(int posX, int posZ)
{
	//Creamos a matriz de transformación
	transform = glm::mat4();
	transform = glm::translate(transform, glm::vec3(posX, -3.0f, posZ));
	transform = glm::scale(transform, glm::vec3((8), (0.1), (8)));

	//Poñemos texturas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textura[1]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textura[1]);
	glUniform3f(corObxectoLoc, 0.496f, 0.425f, 0.396f); //Dámoslle cor

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAOCadrado);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//Función que mostra o chan e a súa cara de atrás
void chan()
{
	//Cara frontal

	//Creamos a matriz de transformación
	transform = glm::mat4();
	transform = glm::translate(transform, glm::vec3(0, 0.0f, 0));
	transform = glm::rotate(transform, (float)(y * graosARadians), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::rotate(transform, (float)(x * graosARadians), glm::vec3(0.0f, 0.0f, 1.0f));
	transformTemporal = transform;
	transform = glm::scale(transform, glm::vec3((4), (0.1), (4)));

	//Poñemos texturas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textura[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textura[0]);
	glUniform3f(corObxectoLoc, 0.933f, 0.8157f, 0.6157f); //Dámoslle cor

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAOCadrado);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//Cara traseira

	//Creamos a matriz de transformación
	transform = transformTemporal;
	transform = glm::translate(transform, glm::vec3(0, -0.12f, 0));
	transform = glm::rotate(transform, (float)(180 * graosARadians), glm::vec3(0.0f, 0.0f, 1.0f));
	transform = glm::scale(transform, glm::vec3((4), (0.1), (4)));

	//Poñemos texturas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textura[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textura[0]);
	glUniform3f(corObxectoLoc, 0.933f, 0.8157f, 0.6157f); //Dámoslle cor

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAOCadrado);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//Función mostra a pantalla de victoria. O movemento desta segue o movemento do plano 
void victoria() {

	//Creamos a matriz de transformación
	transform = transformTemporal;
	transform = glm::translate(transform, glm::vec3(0, 1.0f, 0));
	transform = glm::scale(transform, glm::vec3((4), (0.1), (3)));

	//Poñemos texturas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textura[3]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textura[3]);
	glUniform3f(corObxectoLoc, 1.0f, 1.0f, 1.0f); //Dámoslle cor

	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAOCadrado);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//Función que crea unha esfera de colisión dun obstáculo rotado
void creaEsferaColision(esfera esfera) 
{
	transform = transformTemporal;
	transform = glm::translate(transform, glm::vec3(esfera.px, esfera.py, esfera.pz));
	transform = glm::scale(transform, glm::vec3(esfera.s, esfera.s, esfera.s));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(esfera.vao);
	glDrawArrays(GL_TRIANGLES, 0, esfera.vertices);
}

//Función que calcula as esferas de colisión dos obstáculos rotados
void calculaEsferas(obstaculoRotado *o) {

	int signo;
	esfera *esferas=(esfera*)malloc(o->numEsferas*sizeof(esfera));
	int i = 0;
	float r, distanciaTotal, distanciaEsferas, incremento, posicionx, posicionz;

	//O radio da esfera debe ser a metade do menor escalado (para que non sobresaian as esferas)
	//Na distancia total das esferas deixase un marxe (por iso se suma un no número de esferas) para que tampouco sobresaian polos lados
	if (o->sx > o->sz) {
		r = o->sz/2.0f;
		distanciaTotal = o->sx;
		distanciaEsferas = distanciaTotal / ((float)o->numEsferas +1.0f);
		distanciaTotal -=o->sz;
		signo = -1;
	}
	else {
		r = o->sx/2.0f;
		distanciaTotal = o->sz;
		distanciaEsferas = distanciaTotal / ((float)o->numEsferas +1.0f);
		distanciaTotal -=o->sx;
		signo = 1;
	}
	
	//Calcúlase a posición da primeira esfera
	posicionx = o->px - (distanciaTotal/2* cos(signo*o->rotacion * graosARadians));
	posicionz = o->pz - (distanciaTotal/2 * sin(signo*o->rotacion * graosARadians));

	while (i<o->numEsferas) {

		//Asígnaselle a posición e o radio á esfera
		o->esferas[i].px = posicionx;
		o->esferas[i].py = 0.05;
		o->esferas[i].pz = posicionz;
		o->esferas[i].s = r;
		o->esferas[i].vao = VAOEsfera;
		o->esferas[i].vertices = 1080;
		o->esferas[i].id = 0;

		//Realízase o incremento para debuxar a seguinte esfera
		posicionx += distanciaEsferas *cos(signo*o->rotacion * graosARadians);
		posicionz += distanciaEsferas * sin(signo*o->rotacion * graosARadians);

		i++;
	}
}

//Función que crea un obstáculo
void debuxaObstaculo(obstaculo o)
{
	transform = transformTemporal;
	
	transform = glm::translate(transform, glm::vec3( o.px, o.py,   o.pz));
	transform = glm::scale(transform, glm::vec3(o.sx, o.sy, o.sz));
	glUniform3f(corObxectoLoc, o.R, o.G, o.B); //Dámoslle cor
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(o.vao);
	glDrawArrays(GL_TRIANGLES, 0, o.vertices);
}

//Función que crea un obstáculo rotado
void debuxaObstaculoRotado(obstaculoRotado o)
{
	int i = 0;
	transform = transformTemporal;

	transform = glm::translate(transform, glm::vec3(o.px, o.py, o.pz));
	transform = glm::rotate(transform, (float)(o.rotacion*graosARadians),glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(o.sx, o.sy, o.sz));
	glUniform3f(corObxectoLoc, o.R, o.G, o.B); //Dámoslle cor
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(o.vao);
	glDrawArrays(GL_TRIANGLES, 0, o.vertices);

	//Creamos as esferas de rotación
	for (i = 0; i < o.numEsferas; i++) {
		creaEsferaColision(o.esferas[i]);
	}
}

//Función que debuxa a bola coa que se xoga
void debuxaBola(bola o)
{
	transform = transformTemporal;
	transform = glm::translate(transform, glm::vec3(o.px, o.py, o.pz));
	transform = glm::scale(transform, glm::vec3(o.sx, o.sy, o.sz));
	glUniform3f(corObxectoLoc, o.R, o.G, o.B); //Dámoslle cor
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(o.vao);
	glDrawArrays(GL_TRIANGLES, 0, o.vertices);
}

//Función que debuxa un buraco
void debuxaBuraco(buraco b) {

	transform = transformTemporal;
	transform = glm::translate(transform, glm::vec3(b.px, 0.01f, b.pz));;
	transform = glm::scale(transform, glm::vec3((b.sx), (0.1), (b.sz)));

	//Se o buraco non é a meta
	if (b.id == 100) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textura[2]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textura[2]);
	}
	//Se o buraco é a meta
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textura[4]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textura[4]);
	}

	glUniform3f(corObxectoLoc, b.R, b.G, b.B); //Dámoslle cor
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(b.vao);
	glDrawArrays(GL_TRIANGLES, 0, b.vertices);
}

//Función que crea a grúa e a iluminación
void Display() {

	int i; //Variable de iteración

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //Borramos o buffer da ventana
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, ancho, alto);

	//Debuxamos o fondo
	fondo(-3,0);
	fondo(3, 0);
	fondo(-3, 3);
	fondo(3, 3);
	
	//Debuxamos o chan
	chan();

	//Debuxamos os obstáculos non rotados
	for(i=0;i<numObstaculos;i++)
	{
		obstaculos[i].vao = VAOCubo;
		debuxaObstaculo(obstaculos[i]);
	}

	//Debuxamos os obstáculos rotados
	for (i = 0; i < numObstaculosRotados; i++) {
		obstaculosR[i].vao = VAOCubo;
		calculaEsferas(&obstaculosR[i]);
		debuxaObstaculoRotado(obstaculosR[i]);
	}

	//Debuxamos os buracos
	for (i = 0; i < numBuracos; i++) {
		buracos[i].vao = VAOCadrado;
		debuxaBuraco(buracos[i]);
	}

	//Debuxamos a bola
	b.vao = VAOEsfera;
	debuxaBola(b);

	//Debuxamos a meta
	meta.vao = VAOCadrado;
	debuxaBuraco(meta);

	//Configuración luz
	posLuz = glm::vec4(0.0f, 4.0f, 0.0f, 1.0f);
	direccionLuz = glm::vec3(transform*glm::vec4(0.0f,  0,0.f,1.0f));
	glUniform3f(lightPosLoc, posLuz.x, posLuz.y, posLuz.z);
	glUniform3f(lightDirLoc, direccionLuz.x, direccionLuz.y, direccionLuz.z);
	glUniform3f(lightColorLoc, 1.0f, 0.975f, 0.7f);
	
	glBindVertexArray(0); // unbind
}

//Función que mide o tempo
void tempo()
{
	//Función que mide o tempo para realizar o movemento e que tamén mostra os fps
	static float segundo = 0;
	
	currentTime = glfwGetTime();
	nbFrames++;
	lapsoTime = currentTime - lastTime;
	segundo += lapsoTime;
	
	if (segundo >= 1.0) {
		printf("%f frame /s\n", double(nbFrames));
		nbFrames = 0;
		segundo = 0;

	}
	lastTime = currentTime;
}

//Función que lle aplica as leis da física á bola
void movemento()
{
	//Posición da bola
	float posicionX=b.px;
	float  posicionZ=b.pz;

	//Variable que detecta se houbo choque nunha determinada iteración
	BOOLEAN choque=false;

	//Variables que detectan o tipo de choque que se producíu
	BOOLEAN choqueArriba;
	BOOLEAN choqueAbaixo;

	//Velocidade e aceleración da bola
	velocidadey += lapsoTime * aceleracion;
	if(sin(x * graosARadians)!=0)
	velocidadex += lapsoTime * aceleracion;
	if(sin(y * graosARadians)!=0)
	velocidadez += lapsoTime * aceleracion;

	b.py += lapsoTime * velocidadey;

	//Ademais dos obstáculos dos laterales, poñemos límites para evitar que a bola se vaia do plano
	if (b.px < -1.9)
	{
		velocidadex = 0;
		b.px = -1.9;
	}
	if (b.pz < -1.9)
	{
		velocidadez = 0;
		b.pz = -1.9;
	}
	if (b.px > 1.9)
	{
		velocidadex = 0;
		b.px = 1.9;
	}
	if (b.pz > 1.9)
	{
		velocidadez = 0;
		b.pz = 1.9;
	}

	//A forza da gravidade empuxa a bola contra o chan pero non pode pasar del
	if(b.py<0)
	{
		b.py = 0;
		velocidadey = 0;
		b.px += (sin(x*graosARadians) * lapsoTime * velocidadex)/10;
		b.pz -= (sin(y* graosARadians) * lapsoTime * velocidadez)/10;
	}

	//En cada iteración o choque comeza en false
	choque = false;

	//Por cada obstáculo sen rotar, vemos se a bola está máis cerca del do que mide
	for (obstaculo obs : obstaculos)
	{
		if (abs(b.pz - obs.pz) <= (obs.sz / 2 + b.sz / 2) && abs(b.px - obs.px) <= (obs.sx / 2 + b.sx / 2))
		{
			//Vemos por onde se producía a colisión, movemos lixeiramente a bola para que non entre dentro do obxecto e calculamos o rebote
			if (b.pz > (obs.pz + obs.sz / 2)) {
				velocidadez = -0.15 * velocidadez;
				b.pz = posicionZ+0.005;
			}
			if (b.pz < (obs.pz - obs.sz / 2)) {
				velocidadez = -0.15 * velocidadez;
				b.pz = posicionZ- 0.005;
			}
			if (b.px > (obs.px + obs.sx / 2)) {
				velocidadex = -0.15 * velocidadex;
				b.px = posicionX+ 0.005;
			}
			if (b.px < (obs.px - obs.sx / 2)) {
				velocidadex = -0.15 * velocidadex;
				b.px = posicionX- 0.005;
			}
			//Producíuse un choque
				choque = true;
		}

	}

	//Por cada obstáculo rotado, vemos se a bola está máis cerca del do que miden as bolas que crean as colisións con estes obxectos
	for (obstaculoRotado obs : obstaculosR)
	{
		for (int i = 0; i < obs.numEsferas; i++) {
			if (abs(b.pz - obs.esferas[i].pz) <= 1.2*(obs.esferas[i].s / 2 + b.sz / 2) && abs(b.px - obs.esferas[i].px) <= 1.2*(obs.esferas[i].s / 2 + b.sx / 2)) //Faise a multpiplicación por 1.2 para dar maior realismo, xa que en realidade estase chocando contra esferas
			{
				//Vemos por onde se producíu a colisión, movemos lixeiramente a bola (máis que no caso anterior para dar maior realismo debido a que a forma de detectar as colisións cambia) e calculamos o rebote (máis ca no caso anterior polo mesmo motivo)
				if (b.pz > (obs.esferas[i].pz + obs.esferas[i].s / 2)) {
					velocidadez = 0.6 * velocidadez;
					b.pz = posicionZ + 0.01;
				}
				if (b.pz < (obs.esferas[i].pz - obs.esferas[i].s / 2)) {
					velocidadez = 0.6 * velocidadez;
					b.pz = posicionZ - 0.01;
				}
				if (b.px > (obs.esferas[i].px + obs.esferas[i].s / 2)) {
					velocidadex = 0.6 * velocidadex;
					b.px = posicionX + 0.01;
				}
				if (b.px < (obs.esferas[i].px - obs.esferas[i].s / 2)) {
					velocidadex = 0.6 * velocidadex;
					b.px = posicionX - 0.01;
				}
				//Producíuse un choque
				choque = true;
			}

		}
	}

	//Por cada buraco, vemos se a bola está máis cerca deles do que miden
	for (buraco bur : buracos) {
		//Multiplícanse por 0.55 os escalados porque o buraco en sí non é un cadrado, se non que é unha esfera que está dentro del, polo tanto así temos unha área máis parecida á propia textura do buraco
		if (abs(b.pz - bur.pz) <= 0.55 * (bur.sz / 2 + b.sz / 2) && abs(b.px - bur.px) <= 0.55 * (bur.sx / 2 + b.sx / 2)) {
			//Se houbo colisión, poñemos os distintos elementos na súa posición inicial e volvemos a iniciar o xogo
			b.px = -1.5f;
			b.py = 0.01f;
			b.pz = 1.5f;
			velocidadex = 0.0f;
			velocidadez = 0.0f;
			x = 0.0f;
			y = 0.0f;
		}
	}

	//Detectamos a colisión coa meta de igual forma que os buracos, pero sacouse do bucle anterior para diferenciar mellor o seu tratamento
	if (abs(b.pz - meta.pz) <= (meta.sz / 2 + b.sz / 2) && abs(b.px - meta.px) <= (meta.sx / 2 + b.sx / 2)) {
		//Poñemos a variable de victoria a true
		vi = true;
	}

	//Se non houbo choque, gardamos as posicións da bola, para no caso de que as haia regresar ás posicións xusto anteriores a que se producise o choque
	if (!choque) {
		posicionX = b.px;
		posicionZ = b.pz;
	}

}

//Cámara do xogo
void camara()
{
	//Dámoslle valores ás matrices
	view = glm::lookAt(glm::vec3(0.0f, 4.0f , 0.0f ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	projection = glm::perspective(glm::radians(60.0f), (float)ancho / (float)alto, 0.001f, 20.0f);

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//Posición observador
	glUniform3f(viewPosLoc, (0.0f), 4.0f , 0.0f);
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//Creamos a ventana
	GLFWwindow* window = glfwCreateWindow(ancho, alto, "Laberinto3D", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Fallo Crear Ventana" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: 
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, keyCallBack);
	glfwSetMouseButtonCallback(window, mouse_callback);
	openGlInit();

	//Asignamos os shaders
	shaderProgram = setShaders("shaderPres.vert", "shaderPres.frag");
	
	glUseProgram(shaderProgram);
	
	//Matrices e vectores relativos ós shaders
	transformLoc = glGetUniformLocation(shaderProgram, "transform");
	corObxectoLoc = glGetUniformLocation(shaderProgram, "objectColor");
	lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");
	viewLoc = glGetUniformLocation(shaderProgram, "view");
	projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
	glUniform1i(glGetUniformLocation(shaderProgram, "textureS1"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "textureS2"), 1);

	//Creamos os distintos elementos que imos utilizar para realizar as estruturas do xogo
	debuxaCadrado();
	debuxaCubo();
	debuxaEsfera();

	//Cargamos as texturas
	debuxaTextura("madeira.jpg",0);
	debuxaTextura("terra.jpg", 1);
	debuxaTextura("madeiraBuraco.jpg", 2);
	debuxaTextura("victoria.jpg", 3);
	debuxaTextura("meta.jpg", 4);

	//Facemos a reserva de memoria para as esferas de colisión dos obxectos rotados
	for (int i = 0; i < numObstaculosRotados; i++) {
		obstaculosR[i].esferas = (esfera*)malloc(obstaculosR[i].numEsferas * sizeof(esfera));
	}

	while (!glfwWindowShouldClose(window))
	{
		
		camara();
		Display();
		movemento();
		tempo();

		//Se se producíu unha victoria, lanzamos a ventá de victoria
		if (vi) {
			victoria();
			//Se se pulsa espazo, poñemos os elementos na súa posición inicial e lanzamos de novo o xogo
			if (esPul) {
				b.px = -1.5f;
				b.py = 0.01f;
				b.pz = 1.5f;
				velocidadex = 0.0f;
				velocidadez = 0.0f;
				x = 0.0f;
				y = 0.0f;
				vi = false;
			}
		}
		esPul = false;

		//Se está pulsado o botón esquerdo do rato
		if (pulsado) {

			glfwGetCursorPos(window, &xRato, &yRato);

			//Calcúlase a rotación do plano en función da posición actual do rato e da súa posición previa (para saber canto se moveu)

			if ((yPrev - yRato > 0))
			{
				y += yPrev - yRato;
				yPrev = yRato;
			}
			else if((yPrev - yRato) < 0)
			{
				y += yPrev - yRato;
				yPrev = yRato;
			}
			if ((xPrev - xRato > 0))
			{
				x += xPrev - xRato;
				xPrev = xRato;
			}
			else if ((xPrev - xRato) < 0)
			{
				x+= xPrev - xRato;
				xPrev = xRato;
			}
		}

		//Se se cambiou o tamaño da ventá, realízase un reescalado
		glfwGetWindowSize(window, &ancho, &alto);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Liberamos a memoria reservada dinamicamente
	for (obstaculoRotado obs : obstaculosR) {
		free(obs.esferas);
	}

	glfwTerminate();
	return 0;
}

//Captúrase o movemento do rato se o seu botón esquerdo está pulsado
static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (GLFW_PRESS == action) {
			glfwGetCursorPos(window, &xRato, &yRato);
			yPrev = yRato;
			xPrev = xRato;
			pulsado = true;
		}
		else if (GLFW_RELEASE == action)
			pulsado = false;
	}
}

//Captúrase o movemento dalgunhas teclas
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_SPACE) {
			esPul = true;
	}
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}