#include <cstdlib>
/*#include <sys/time.h>*/
#include <chrono>
#include <ctime>
/*#include <unistd.h>*/
#include <string>



#define MAX(i,j) i>j ? i : j
#define MIN(i,j) i>j ? j : i

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#ifdef HAVE_NCURSES
#include <ncurses.h>

typedef struct win{
  WINDOW* pwin;
  int* height;
  int* width;
  int  starty;
  int  startx;
} win;

typedef struct point{
  int x;
  int y;
} point;

typedef struct map{
  int   height;
  int   width;
  win   window;
  point apple;
} map;

typedef struct snake{
  bool   growing;
  bool   alive;
  int    eaten;
  int    length;
  int    head;
  int    tail;
  int    colorPair;
  long int speed;
  point* position;
} snake;

void winCreate(win* localWin, int* height, int* width, int starty, int startx);
void winDestruct(win* localWin);
void mapInit(map* localMap, int height, int width, int starty, int startx);
void mapNewApple(map* localMap);
inline void mapPrintApple(map* localMap);
void mapDestruct(map* localMap);
void snakeInit(snake* localSnake, int length, point startingPoint, map* plateau, int color);
inline void snakePrint(snake*, int, map*);
inline void snakeErase(snake*, int, map*);
inline void snakePrintOne(snake*, map*);
inline void snakeEraseOne(snake*, map*);
void snakeMove(snake* snake1, map* plateau, int direction);
bool snakeCollision(snake* localSnake, map* plateau);
bool snakeEat(snake* localSnake, point* apple);
bool snakeIsSnake(snake* snakes, int nbJoueurs, point* apple);
void snakeDestruct(snake* localSnake);

int runSnake() {
  int height, width;
  /*
     int direction = -2;
     int previousDirection = -2;
     */
  int* p_direction = NULL;
  int* p_previousDirection = NULL;
  std::string joueurDir[3];
  joueurDir[0] = "<^v>";
  joueurDir[1] = "qzsd";
  joueurDir[2] = "4856";
  int survivants = 0;
  int level = 1; int ch;


  std::chrono::system_clock::time_point timeOld;
  std::chrono::system_clock::time_point timeNew;

  map plateau;
  point startingPoint;
  /*
     snake snake1;
     */
  snake* p_snakes = NULL;

  srand(time(NULL));
  initscr();
  clear();

  if(has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }

  start_color();    
  init_pair(1,COLOR_RED,COLOR_BLACK);
  attron(COLOR_PAIR(1));
  mvprintw(0,COLS/2-11,"**********************");
  mvprintw(1,COLS/2-11,"***     iSnake     ***");
  mvprintw(2,COLS/2-11,"***     v 0.2.1    ***");
  mvprintw(3,COLS/2-11,"**********************\n");
  attroff(COLOR_PAIR(1));
  mvprintw(LINES-1,0,"Quitter : Q");
  refresh();

  height = MIN(LINES-6,20);
  width  = 2*height; 

  int nbJoueurs = 0;
  echo();
  char message1[]="Entrer le nombre de joueurs : ";
  do {
    mvprintw(4,0,"%s",message1);
    mvprintw(4,30," ");
    refresh();
    move(4,30);
    refresh();
    char format[] = "%d";
    mvscanw(4,30,format,&nbJoueurs);
  }while(nbJoueurs < 0 || nbJoueurs > 3);

  noecho();
  cbreak();
  timeout(0);
  curs_set(0);
  keypad(stdscr,TRUE);

  p_direction = (int*) malloc(nbJoueurs*sizeof(int));
  p_previousDirection = (int*) malloc(nbJoueurs*sizeof(int));
  p_snakes = (snake*) malloc(nbJoueurs*sizeof(snake));

  survivants = nbJoueurs;

  srand(time(NULL));

  mapInit(&plateau, height,width,6,(COLS-width)/2);
  mvwprintw(plateau.window.pwin,0,(plateau.width-10)/2," Level %d ",level);
  int nb;
  for (nb=0;nb<nbJoueurs;nb++){
    startingPoint.x = (rand() % (width-2))+1;
    startingPoint.y = (rand() % (height-2))+1;
    snakeInit(&p_snakes[nb],4, startingPoint, &plateau, nb+2);
    p_direction[nb] = -2; //rand()%4-1;
    init_pair(nb+2, nb+3, COLOR_BLACK);
    attron(COLOR_PAIR(nb+2));
    mvprintw(5+nb,0,"Joueur %d : %s",nb+1,joueurDir[nb].c_str());
    attroff(COLOR_PAIR(nb+2));
  }
  snakePrint(p_snakes,nbJoueurs,&plateau);
  refresh();
  do{
    mapNewApple(&plateau);
  } while(snakeIsSnake(p_snakes, nbJoueurs, &(plateau.apple)));
  mapPrintApple(&plateau);

  ch = KEY_LEFT;
  set_escdelay(25);
  while(ch != 'Q' && survivants >= 1) {
    std::chrono::duration<long int,std::micro> delay(p_snakes[0].speed);
    timeOld = std::chrono::system_clock::now();
    timeNew = std::chrono::system_clock::now()-delay;
    while( (timeNew<timeOld) && (ch=getch())!='Q' ) {
      switch(ch) {
        case KEY_UP :
          if (nbJoueurs >= 1) p_direction[0] = -1;
          break;
        case KEY_DOWN :
          if (nbJoueurs >= 1) p_direction[0] = 1;
          break;
        case KEY_RIGHT :
          if (nbJoueurs >= 1) p_direction[0] = 2;
          break;
        case KEY_LEFT :
          if (nbJoueurs >= 1) p_direction[0] = -2;
          break;
        case 'z' :
          if (nbJoueurs >= 2) p_direction[1] = -1;
          break;
        case 's' :
          if (nbJoueurs >= 2) p_direction[1] = 1;
          break;
        case 'd' :
          if (nbJoueurs >= 2) p_direction[1] = 2;
          break;
        case 'q' :
          if (nbJoueurs >= 2) p_direction[1] = -2;
          break;
        case '8' :
          if (nbJoueurs >= 3) p_direction[2] = -1;
          break;
        case '5':
          if (nbJoueurs >= 3) p_direction[2] = 1;
          break;
        case '6' :
          if (nbJoueurs >= 3) p_direction[2] = 2;
          break;
        case '4' :
          if (nbJoueurs >= 3) p_direction[2] = -2;
          break;
      }
      timeNew = std::chrono::system_clock::now()-delay;
    }

    snakeErase(p_snakes,nbJoueurs,&plateau);

    for(nb=0; nb<nbJoueurs; nb++){
      if( p_previousDirection[nb] == -p_direction[nb] )
        p_direction[nb] = p_previousDirection[nb];
      p_previousDirection[nb] = p_direction[nb];
      snakeMove(&p_snakes[nb],&plateau,p_direction[nb]);
      if(snakeEat(&p_snakes[nb],&(plateau.apple))){ 
        if ( p_snakes[nb].eaten == 0 ) {
          level++;
          mvwprintw(plateau.window.pwin,0,(plateau.width-10)/2," Level %d ",level);
        }	
        do{
          mapNewApple(&plateau);
        } while(snakeIsSnake(p_snakes, nbJoueurs, &plateau.apple));
        mapPrintApple(&plateau);
      }
    }

    snakePrint(p_snakes,nbJoueurs,&plateau);

    for(nb=0; nb<nbJoueurs; nb++){
      if(snakeCollision(&p_snakes[nb],&plateau)){
        timeout(500);
        snakeEraseOne(&p_snakes[nb],&plateau);
        getch();
        snakePrintOne(&p_snakes[nb],&plateau);
        getch();
        snakeEraseOne(&p_snakes[nb],&plateau);
        getch();
        snakePrintOne(&p_snakes[nb],&plateau);
        getch();
        snakeEraseOne(&p_snakes[nb],&plateau);
        getch();
        snakePrintOne(&p_snakes[nb],&plateau);
        mvprintw(5,COLS/2-9,"Joueur %d a perdu !",nb+1);
        snakeEraseOne(&p_snakes[nb],&plateau);
        refresh();
        timeout(1000);
        getch();
        mvprintw(5,COLS/2-9,"                  ");
        refresh();
        timeout(0);
        survivants--;
      }
    }
  }
  snakeErase(p_snakes,nbJoueurs,&plateau);
  mapDestruct(&plateau);
  for(nb=0;nb<nbJoueurs;nb++){
    snakeDestruct(&p_snakes[nb]);
  }
  free(p_direction);
  free(p_previousDirection);
  free(p_snakes);
  refresh();
  endwin();
  return 0;
}

void winCreate(win* localWin, int* height, int* width, int starty, int startx) {

  localWin->height = height;
  localWin->width = width;
  localWin->starty = starty;
  localWin->startx = startx;
  localWin->pwin = newwin(*height, *width, starty, startx);
  wborder( localWin->pwin,ACS_VLINE,ACS_VLINE,ACS_HLINE,ACS_HLINE,ACS_ULCORNER,ACS_URCORNER,ACS_LLCORNER,ACS_LRCORNER);
  wrefresh(localWin->pwin);
}	

void winDestruct(win* localWin){
  wborder(localWin->pwin, ' ',' ',' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(localWin->pwin);
  delwin(localWin->pwin);
}

void mapInit(map* localMap, int height, int width, int starty, int startx) {
  localMap->height = height;
  localMap->width  = width;
  winCreate(&(localMap->window), &(localMap->height), &(localMap->width), starty, startx);
}

void mapNewApple(map* localMap) {
  int newX;
  int newY;
  int moduloX;
  int moduloY;
  moduloX = localMap->width -2;
  moduloY = localMap->height-2;
  newX = rand()%moduloX+1;
  newY = rand()%moduloY+1;
  localMap->apple.x = newX;
  localMap->apple.y = newY;
}

inline void mapPrintApple(map* localMap) {
  mvwaddch(localMap->window.pwin,localMap->apple.y,localMap->apple.x,ACS_DIAMOND);
  wrefresh(localMap->window.pwin);
}

void mapDestruct(map* localMap) {
  mvwaddch(localMap->window.pwin,localMap->apple.y,localMap->apple.x,' ');
  refresh();
  winDestruct(&(localMap->window));
}

void snakeInit(snake* localSnake, int length, point startingPoint,map* plateau, int color) {
  int i;
  localSnake->length = length;
  localSnake->position = (point*) calloc(length,sizeof(point));
  localSnake->colorPair = color;
  for( i = 0; i<length; i++){
    localSnake->position[i].y = startingPoint.y;
    localSnake->position[i].x = startingPoint.x+i > plateau->width - 2 ? 
      startingPoint.x+i-plateau->width + 2 : startingPoint.x+i;
  }
  localSnake->alive   = true;
  localSnake->growing = false;
  localSnake->head    = 0;
  localSnake->tail    = length-1;
  localSnake->speed   = 500000;
  localSnake->eaten   = 0;
}

inline void snakePrint(snake* snakes, int nbJoueurs, map* plateau){
  int i;
  int j;
  for (j=0;j<nbJoueurs;j++){
    if ( snakes[j].alive ) {
      wattron(plateau->window.pwin,COLOR_PAIR(snakes[j].colorPair));
      for( i=0;i<snakes[j].length;i++){
        mvwaddch(plateau->window.pwin,snakes[j].position[i].y,snakes[j].position[i].x, ACS_CKBOARD);
      }
      wattroff(plateau->window.pwin,COLOR_PAIR(snakes[j].colorPair));
    }
  }
  wrefresh(plateau->window.pwin);
}

inline void snakeErase(snake* snakes, int nbJoueurs, map* plateau){
  int i;
  int j;
  for (j=0;j<nbJoueurs;j++){
    if ( snakes[j].alive ) {
      for( i=0;i<snakes[j].length;i++){
        mvwaddch(plateau->window.pwin,snakes[j].position[i].y,snakes[j].position[i].x, ' ');
      }
    }
  }
  wrefresh(plateau->window.pwin);
}

inline void snakePrintOne(snake* snake, map* plateau){
  int i;
  wattron(plateau->window.pwin,COLOR_PAIR(snake->colorPair));
  for( i=0;i<snake->length;i++){
    mvwaddch(plateau->window.pwin,snake->position[i].y,snake->position[i].x, ACS_CKBOARD);
  }
  wattroff(plateau->window.pwin,COLOR_PAIR(snake->colorPair));
  wrefresh(plateau->window.pwin);
}

inline void snakeEraseOne(snake* snake, map* plateau){
  int i;
  for( i=0;i<snake->length;i++){
    mvwaddch(plateau->window.pwin,snake->position[i].y,snake->position[i].x, ' ');
  }
  wrefresh(plateau->window.pwin);
}
bool snakeIsSnake(snake* snakes, int nbJoueurs, point* apple) {
  int i;
  int j;
  for (j=0;j<nbJoueurs;j++){
    for (i=0;i<snakes[j].length;i++){
      if( snakes[j].position[i].x == apple->x && snakes[j].position[i].y==apple->y)
        return true;
    }
  }
  return false;
}

void snakeMove(snake* snake1, map* plateau, int direction) {
  int i,j;
  int head;
  int tail;
  int newHead;
  int newTail;
  int variation;
  point* newPosition = NULL;

  //	for(i=snake1->length;i > 0;i--)
  //		snake1->position[i] = snake1->position[i-1];
  head = snake1->head;
  tail = snake1->tail;
  if( snake1->growing ) {
    newPosition = (point*) calloc(snake1->length+1,sizeof(point));
    for( i=0,j=0;i<snake1->length;i++,j++) {
      newPosition[j].x = snake1->position[i].x;
      newPosition[j].y = snake1->position[i].y;
      if( i == tail ) {
        newTail = i;
        switch (direction){
          case 1 :
            variation = snake1->position[head].y + 1;
            newPosition[++j].x = snake1->position[head].x;
            newPosition[j].y = variation == plateau->height-1 ? 1 : variation;
            break;
          case -1 :
            variation = snake1->position[head].y - 1;
            newPosition[++j].x = snake1->position[head].x;
            newPosition[j].y = variation == 0 ? plateau->height - 2 : variation;
            break;
          case 2:
            variation = snake1->position[head].x + 1;
            newPosition[++j].y = snake1->position[head].y;
            newPosition[j].x = variation == plateau->width-1 ? 1 : variation;
            break;
          case -2:
            variation = snake1->position[head].x - 1;
            newPosition[++j].y = snake1->position[head].y;
            newPosition[j].x = variation == 0 ? plateau->width - 2 : variation;
            break;
        }
        newHead = j;
      }
    }
    ++snake1->length;
    snake1->position = newPosition;
  }
  else {
    switch (direction){
      case 1 :
        variation = snake1->position[head].y + 1;
        snake1->position[tail].x = snake1->position[head].x;
        snake1->position[tail].y = variation == plateau->height-1 ? 1 : variation;
        break;
      case -1 :
        variation = snake1->position[head].y - 1;
        snake1->position[tail].x = snake1->position[head].x;
        snake1->position[tail].y = variation == 0 ? plateau->height - 2 : variation;
        break;
      case 2:
        variation = snake1->position[head].x + 1;
        snake1->position[tail].y = snake1->position[head].y;
        snake1->position[tail].x = variation == plateau->width-1 ? 1 : variation;
        break;
      case -2:
        variation = snake1->position[head].x - 1;
        snake1->position[tail].y = snake1->position[head].y;
        snake1->position[tail].x = variation == 0 ? plateau->width - 2 : variation;
        break;
    }
    newHead = snake1->tail;
    newTail = snake1->tail == 0 ? snake1->length-1 : snake1->tail-1;
  }
  snake1->head = newHead;
  snake1->tail = newTail;
  /*
     for(i=0;i<snake1->length;i++) {
     mvprintw(30,5*i+4,"%d / ",snake1->position[i].x);
     mvprintw(31,5*i+4,"%d / ",snake1->position[i].y);
     }
     refresh();
     */
}

bool snakeCollision(snake* localSnake, map* plateau){
  int head;
  int i;
  head = localSnake->head;
  for( i=0;i<localSnake->length;i++) {
    if(i!=head) {
      if(localSnake->position[head].x == localSnake->position[i].x
          && localSnake->position[head].y == localSnake->position[i].y){
        localSnake->alive = false;
        return true;
      }
    }
  }
  return false;
}

bool snakeEat(snake* localSnake, point* apple) {
  if(localSnake->position[localSnake->head].x == apple->x
      && localSnake->position[localSnake->head].y == apple->y ) {
    localSnake->growing = true;
    if ( ++(localSnake->eaten) == 2 ) {
      localSnake->eaten = 0;
    }
    localSnake->speed  = (long int) (localSnake->speed * 0.96);
    return true;
  }
  else {
    localSnake->growing = false;
    return false;
  }
}

void snakeDestruct(snake* localSnake){
  free(localSnake->position);
}

#else
int runSnake() {return 0;}
#endif
