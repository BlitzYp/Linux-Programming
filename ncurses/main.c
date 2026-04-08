#include <ncurses.h>
#include <unistd.h>
#include <math.h>

void circle(float x,float y,float r);
void game();

int main(void)
{
    initscr();
    noecho(); // disable buttons showing
    curs_set(FALSE);
    game();
    /*
    for (;;) {
        clear();
        circle(8.,8.,4.);
        usleep(50000);
        refresh();
    }
    */
    endwin();
    return 0;
}

void circle(float x,float y,float r)
{
    // x^2+y^2<r^2
    // cx=cos(), cy=sin() unit circle,we also multiply with r
    float cx,cy;
    float angle=0.0;
    float da=0.1;
    clear();
    for (float angle=0.;angle<=2*M_PI;angle+=da) {
        cx=cos(angle),cy=sin(angle);
        mvprintw(r*cy+y,r*cx+x,"o");
    }
    refresh();
}

void game()
{
    float x=1.0,y=1.0,r=1.0;
    float dx=0.3,dy=0.3,dr=0.5;
    float sx=1.0,sy=1.0,sr=1.0;
    for (;;) {
        circle(x,y,r);
        usleep(50000);
        x+=sx*dx;
        y+=sy*dy;
        r+=sr*dr;
        if (x>=40.) x=1.;
        if (y>=20.) y=1.;
        if (r>=10.) r=1.;
    }
}
