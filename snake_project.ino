#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>

//GAME STATES
#define START 0
#define GAMEOVER 1
#define PLAYING 2

//GAME CLASSES
class MatrixMap;
class Panel;
class Snake;
class Apple;
class Game;

//SNAKE DIRECTIONS
#define STOPPED 0
#define RIGHT 1
#define LEFT 2
#define TOP 3
#define BOTTOM 4

/*
============================
         MAP CLASS
============================
*/
class MatrixMap{

public:
    int rows;
    int columns;
    Adafruit_NeoPixel **matrix;

    MatrixMap() {}

    MatrixMap(int rows, int columns)
    {
        this->rows = rows;
        this->columns = columns;
        matrix = new Adafruit_NeoPixel *[rows];
        initMatrix();
    }

    void initMatrix(){
        for (int i = 0, j = 13; i < rows; ++i, --j)
        {
            matrix[i] = new Adafruit_NeoPixel(columns, j, NEO_GRB + NEO_KHZ800);
            matrix[i]->begin();
        }
    }

    void setLED(int column, int row, int r, int g, int b){
        matrix[row]->setPixelColor(column, r, g, b);
    }

    void clear(){
        for (int i = 0; i < rows; i++)
            matrix[i]->clear();
    }

    void show(){
        for (int i = 0; i < rows; i++)
            matrix[i]->show();
    }

    ~MatrixMap(){}
};

/*
============================
       PANEL CLASS
============================
*/

//LCD
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

class Panel{

public:
    static void startPanel(){
        lcd.setCursor(3, 0);
        lcd.write("SNAKE GAME");

        lcd.setCursor(0, 1);
        lcd.write("PRESS ANY BUTTON TO START");
    }

    static void playingPanel(int score, int difficult){
        lcd.setCursor(0, 0);
        lcd.write("YOUR SCORE:");
        lcd.print(score);

        lcd.setCursor(0, 1);
        lcd.write("DIFFICULT:");
        lcd.print(difficult);
    }

    static void losePanel(int score, int record, bool newRecord){
        lcd.setCursor(0, 0);
        lcd.write("Current score:");
        lcd.print(score);

        lcd.setCursor(0, 1);
        if (newRecord)
            lcd.write("New record:");
        else
            lcd.write("Your record:");

        lcd.print(record);

        //ALTERNATES BETWEEN PANELS
        delay(2000);

        lcd.clear();
        startPanel();

        delay(2000);

        lcd.clear();
        lcd.setCursor(0, 4);
        lcd.write("You lose!");

        delay(2000);

        lcd.clear();
    }
};

/*
================================
         SOUND EFFECTS
================================
*/
void pointSound(){
    tone(5, 988, 100);
    delay(80);
    tone(5, 1319, 850);
    delay(80);
    noTone(5);
}

void loseSound(){
    tone(5, 392);
    delay(250);
    tone(5, 392);
    delay(500);
    noTone(5);
}

/*
================================
     APPLE AND SNAKE CLASSES
================================
*/

#define MAX_SNAKE_LENGTH 30
typedef struct{
    int x;
    int y;
} Node;

class Apple{

private:
    Node position;
    friend class Game;

public:
    Apple() {}

    Apple(int x, int y){
        position.x = x;
        position.y = y;
    }

    ~Apple() {}
};

class Snake{

private:
    Node body[MAX_SNAKE_LENGTH];
    int length;
    int direction;
    int score;
    int record;
    bool newRecord;
    friend class Game;

public:
    Snake() {}

    Snake(int x, int y){
        #define head body[0]
        body[0].x = x;
        body[0].y = y;
        score = 0;
        record = 0;
        direction = STOPPED;
        length = 1;
    }

    void setDirection(int dir){

        if (dir == RIGHT)
            direction = (direction == RIGHT) ? BOTTOM : 
                        (direction == LEFT) ? TOP : dir;
        else if (dir == LEFT)
            direction = (direction == LEFT) ? BOTTOM : 
            (direction == RIGHT) ? TOP: dir;
    }

    ~Snake() {}
};

/*
============================
       GAME CLASS
============================
*/
class Game{

public:
    Snake snake;
    Apple apple;
    MatrixMap matrixMap;
    int difficult;
    int state;

    Game(){
        int xSnake = random(0, 12);
        int ySnake = random(0, 7);

        int xApple = random(0, 12);
        int yApple = random(0, 7);
        while (xApple == xSnake && yApple == ySnake)
        {
            xApple = random(0, 12);
            yApple = random(0, 7);
        }

        MatrixMap matrixMap(7, 12);
        Apple ap(xApple, yApple);
        Snake sn(xSnake, ySnake);

        this->snake = sn;
        this->apple = ap;

        #define map matrixMap.matrix
        this->matrixMap = matrixMap;

        difficult = 0;
        state = START;
    }

    void restartGame(){
        snake.direction = STOPPED;
        snake.length = 1;
        snake.newRecord = false;
        snake.score = 0;
        state = PLAYING;
        difficult = 0;

        int xSnake = random(0, 12);
        int ySnake = random(0, 7);

        int xApple = random(0, 12);
        int yApple = random(0, 7);

        while (xApple == xSnake && yApple == ySnake){
            xApple = random(0, 12);
            yApple = random(0, 7);
        }

        snake.head.x = xSnake;
        snake.head.y = ySnake;
        apple.position.x = xApple;
        apple.position.y = yApple;
    }

    void generateApple(){
        int xApple = random(0, 12);
        int yApple = random(0, 7);

        for (int i = 0; i < snake.length; i++){
            while (xApple == snake.body[i].x && yApple == snake.body[i].y){
                xApple = random(0, 12);
                yApple = random(0, 7);
            }
        }

        apple.position.x = xApple;
        apple.position.y = yApple;
    }

    void appleEaten(){
        if (apple.position.x == snake.head.x && apple.position.y == snake.head.y){
            snake.length++;
            snake.body[snake.length - 1].x = snake.head.x;
            snake.body[snake.length - 1].y = snake.head.y;
            snake.score++;

            if (300 - difficult > 50)
                difficult += 15;

            generateApple();

            pointSound();
        }
    }

    void gameOver(){
        for (int i = 0; i < matrixMap.rows; i++){
            map[i]->clear();
            delay(100);
            map[i]->show();
        }

        state = GAMEOVER;

        if (snake.score > snake.record){
            snake.record = snake.score;
            snake.newRecord = true;
        }

        loseSound();
    }

    void checkCollision(){
        for (int i = snake.length; i > 2; i--){
            if (snake.head.x == snake.body[i].x &&
                snake.head.y == snake.body[i].y){

                snake.direction = STOPPED;
                gameOver();
            }
        }
    }

    void displayPanelInfo(){
        lcd.clear();

        if (state == START)
            Panel::startPanel();
        else if (state == PLAYING)
            Panel::playingPanel(snake.score, difficult);
        else
            Panel::losePanel(snake.score, snake.record, snake.newRecord);
    }

    void update(){

        displayPanelInfo();

        if (snake.direction != STOPPED){

            //UPDATE SNAKE BODY POSITION
            for (int i = snake.length - 1; i > 0; --i){
                snake.body[i].x = snake.body[i - 1].x;
                snake.body[i].y = snake.body[i - 1].y;
            }

            //INCREMENT/DECREMENT SNAKE POSITION
            int direction = snake.direction;
            int x = snake.head.x, y = snake.head.y;

            if (direction == RIGHT)
                snake.head.x = (x < 11) ? snake.head.x + 1 : 0;
            else if (direction == LEFT)
                snake.head.x = (x > 0) ? snake.head.x - 1 : 11;
            else if (direction == BOTTOM)
                snake.head.y = (y < 6) ? snake.head.y + 1 : 0;
            else if (direction == TOP)
                snake.head.y = (y > 0) ? snake.head.y - 1 : 6;

            checkCollision();
        }

        if (state != GAMEOVER){

            //TURN ON APPLE LED
            matrixMap.setLED(apple.position.x, apple.position.y, 255, 0, 0);

            //TURN ON SKANE LEDS
            for (int i = 0; i < snake.length; i++)
                matrixMap.setLED(snake.body[i].x, snake.body[i].y, 0, 255, 0);
        }

        appleEaten();

        matrixMap.show();
    }

    ~Game(){};
};

//GLOBAL GAME INSTANCE
Game game;

//INTERRUPTION FUNCTIONS
static void updateRight(){
    if (game.state != GAMEOVER){
        game.snake.setDirection(RIGHT);
        game.state = PLAYING;
    } else
        game.restartGame();
}

static void updateLeft(){
    if (game.state != GAMEOVER){
        game.state = PLAYING;
        game.snake.setDirection(LEFT);
    } else
        game.restartGame();
}

//ARDUINO FUNCTIONS
void setup(){
    Serial.begin(9600);
    randomSeed(analogRead(0));
    pinMode(5, OUTPUT);
    lcd.begin(16, 2);

    volatile uint8_t leftBtn = 2;
    volatile uint8_t rightBtn = 3;

    pinMode(leftBtn, INPUT_PULLUP);
    pinMode(rightBtn, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(leftBtn), updateLeft, RISING);
    attachInterrupt(digitalPinToInterrupt(rightBtn), updateRight, RISING);
}

void loop(){
    game.matrixMap.clear();
    game.update();
    delay(300 - game.difficult);
}
