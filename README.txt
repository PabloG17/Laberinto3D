*****************************************************************************Laberinto3D*****************************************************************************

O xogo consiste en mover unha bola por un plano con distintos obstáculos que crean un laberinto, e no que hai distintos buracos que hai que evitar para chegar a meta.
O plano móvese mediante o uso do rato, pulsando o botón dereito e desprazándoo.
Se se cae nun buraco o xogo reiníciase, se se chega a meta sae a pantalla de victoria.


*****************************************************************************Construción********************************************************************************

O xogo foi construído en OpenGL 3.3

Vaise pasar a detallar como se realizaron os distintos elementos

Fondo:

Un fondo que está detrás do plano que movemos. Ten unha textura asociada. Crease coa función fondo().

Chan:

É simplemente un cadrado cunha textura. Crease coa función chan().

Obstáculos rectos:

Son simples rectángulos con distinto escalado. Hai un vector no que están todos gardados e créanse cun bucle e coa función debuxaObstaculo()

Obstáculos rotados:

Son obstáculos coma os anteriores, pero creanse de xeito distinto porque están rotados e teñen un tratamento distintos de colisións. Hai un novo vector no que se
almacenan e dende o que se crean coa función creaObstaculoRotado(). Non se usa o mesmo que no caso dos obstáculos rectos porque o efecto das colisións é moi distinto,
xa que se fan mediante esferas que hai no seu interior e que se crean coa función creaEsferaColision(), despois de que a súa posición sexa calculada coa función 
calculaEsferas().

Buracos:

Son cadrados posicionados no chan cos que non debe colisionar a bola. Creanse coa función debuxaBuraco(). Teñen unha textura igual que a do chan pero con un oco para
ser visualizados dunha forma intuitiva.

Bola:

Bola que hai que mover polo plano. É unha esfera creada coa función debuxaBola(). Está baixo os efectos da gravidade e vai tender a ir hacia a posición na que se xire
o plano collendo cunha certa aceleración.

Colisións (Unha das partes máis importantes do xogo):

Detéctase cando un obxecto choca con outro no momento no que a posición entre os seus centros é igual ou menor á metade do seu escalado nesa dirección. O efecto ó
detectar unha colisión é non deixarlle avanzar á bola nesa dirección e dar un pequeno efecto de rebote no outra.

No caso dos obxectos rotados, non se pode detectar a colisión a través do seu escalado, polo que estes están recheos de esferas coas que si se detecta a colisión
coma no caso anterior. Mais ó non ser unha estrutura recta, o efecto ó deslizarse sobre unha destas superficies non é 100% realista, se non que se sinte unha pequena
vibración.

Cámara:

A cámara está fixa nun punto, xa que non se atoparon razóns para cambiala de sitio. Está en perspectiva dende unha certa altura no eixo y.

Iluminación:

Existe un foco na posición da cámara que ilumina a escena. Baixouse o efecto da luz nos shaders nunha pequena cantidade para que non brillen demasiado os elementos
que a reciben de frente.