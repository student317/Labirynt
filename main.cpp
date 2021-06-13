#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <term.h>
#include <unistd.h>
#include <curses.h>
#include <vector>
#include <cstdlib>

using namespace std;

enum elementymapy {gracznadrabinie=0+'%', powietrze= 0+' ',sciana = 0 + '#', drabina_dol = 0 +'v', drabina_gora = 0+'^', koniec=0 +'M',gracz = 0 + '@',mgla=0+'?' };
enum sterowanie {menu= 0+'m',lewo= 0 + 'a',prawo=0+'d',gora= 0+'w',dol= 0+'s',akcja=0+'x'};

///fajna obsługa klawiatury z internetu

static struct termios initial_settings, new_settings;
static int peek_character = -1;

int kbhit()
{
    char ch;
    int nread;

    if(peek_character != -1)
        return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);

    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}

int readch()
{
    char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

///koniec kodu z internetu

class pozycja_gracza
{
public:
    int x;
    int y;

    bool czyna(int s,int w)
    {
        return w==y&&s==x;
    }

    pozycja_gracza()
    {
        x=0;
        y=0;
    }

};

class mapa
{
public:
    int szerokosc;
    int wysokosc;
    char tablica[55][55];
    pozycja_gracza *pozycja;

    void wstaw_gracza(pozycja_gracza *a)
    {
        pozycja=a;
    }

    mapa(int s,int w)
    {
        szerokosc=s;
        wysokosc=w;
        for(int i=0; i<wysokosc+2; i++)
            for(int j=0; j<szerokosc; j++)
                tablica[j][i]=sciana;
/*
        tablica[2][2]=drabina_dol;
        tablica[3][3]=drabina_gora;
        tablica[4][4]=koniec;*/
    }

    virtual void rysuj()
    {
        // system("clear"); poniżej "jakaś" szybsza alternatywa tej komendy
        cout << "\033[2J\033[1;1H";
        for(int j =0; j<szerokosc+2; j++)cout << (char)sciana;
        cout <<endl;
        for(int i=0; i<wysokosc; i++)
        {
            cout << (char)sciana;
            for(int j=0; j<szerokosc; j++)
            {
                if(pozycja->czyna(j,i))
                {
                    if(tablica[j][i]!=powietrze)cout << (char)gracznadrabinie;
                    else cout << (char)gracz;
                }
                else cout << tablica[j][i];
            }
            cout << (char)sciana<<endl;
        }
        for(int j=0; j<szerokosc+2; j++)cout << (char)sciana;
        cout <<endl;

    }


};

class wizja1 : public mapa
{
public:
    wizja1(int s,int w) :mapa(s,w) {}

    virtual void rysuj()
    {
        // system("clear"); poniżej "jakaś" szybsza alternatywa tej komendy
        cout << "\033[2J\033[1;1H";

        int s = pozycja->x -1;
        int w = pozycja->y -1;

        for(int i =0; i<3; i++)
        {
            for(int j =0; j<3; j++)
            {
                if(i+w>=wysokosc|| i+w<0 ||j+s>=szerokosc|| j+s<0)cout << (char)sciana;
                else if(pozycja->czyna(j+s,i+w))
                {
                    if(tablica[j+s][i+w]!=powietrze)cout << (char)gracznadrabinie;
                    else cout << (char)gracz;
                }
                else cout << tablica[j+s][i+w];
            }
            cout <<endl;
        }
    }

};

class wizja2 : public mapa
{
public:
    bool odkryte [55][55];
    wizja2(int s,int w) :mapa(s,w)
    {
        for(int j =0; j<szerokosc; j++)
            for(int i=0; i<wysokosc; i++)
                odkryte[j][i]=0;
    }

    void odkryj()
    {
        int s = pozycja->x -1;
        int w = pozycja->y -1;
        for(int i =0; i<3; i++)
            for(int j =0; j<3; j++)
                if(i+w>wysokosc|| i+w<0 ||j+s>szerokosc|| j+s<0);
                else odkryte[j+s][i+w]=1;
    }


    virtual void rysuj()
    {
        // system("clear"); poniżej "jakaś" szybsza alternatywa tej komendy
        cout << "\033[2J\033[1;1H";
        odkryj();
        for(int j =0; j<szerokosc+2; j++)cout << (char)sciana;
        cout <<endl;
        for(int i=0; i<wysokosc; i++)
        {
            cout << (char)sciana;
            for(int j=0; j<szerokosc; j++)
            {
                if(pozycja->czyna(j,i))
                {
                    if(tablica[j][i]!=powietrze)cout << (char)gracznadrabinie;
                    else cout << (char)gracz;
                }
                else if(!odkryte[j][i])cout << (char)mgla;
                else cout << tablica[j][i];
            }
            cout << (char)sciana<<endl;
        }
        for(int j=0; j<szerokosc+2; j++)cout << (char)sciana;
        cout <<endl;
    }


};



class ruch;
class menugry;

class ustawienia
{
public:
    menugry *mny;
    mapa *plansza[3];
    int poziom;
    int wieklkoscmapyX;
    int wieklkoscmapyY;
    ruch *ruszaj;
    ustawienia(int x,int y,menugry *z)
    {
        mny=z;
        wieklkoscmapyX =x;
        wieklkoscmapyY =y;
        plansza[0]= new wizja1(wieklkoscmapyX,wieklkoscmapyY);
        plansza[0]->wstaw_gracza(new pozycja_gracza());
        plansza[1]= new wizja2(wieklkoscmapyX,wieklkoscmapyY);
        plansza[1]->wstaw_gracza(new pozycja_gracza());
        plansza[2]= new mapa(wieklkoscmapyX,wieklkoscmapyY);
        plansza[2]->wstaw_gracza(new pozycja_gracza());
        poziom=1;
    }

    void funkcje(int jaka);


};

class pozycja_w_menu
{
public:
    vector<string> tresc;
    int stan;
    int ile;
    pozycja_w_menu(vector<string> p)
    {
        tresc=p;
        stan=0;
        ile=p.size();
    }
    void zmienStan()
    {
        stan=(stan+1 )%ile;
    }

    string  tekst()
    {
        return tresc[stan];
    }

};


class menugry
{
public:
    int kursor;
    int trwa;
    vector<pozycja_w_menu*> obcje;
    ustawienia * ust;
    menugry()
    {
        trwa=1;
        ust= new ustawienia(15,15,this);
        kursor=0;
        vector<string> A;
        A.push_back("Start");
        A.push_back("Wznów");
        obcje.push_back( new pozycja_w_menu(A));
        vector<string> B;
        B.push_back("Rozmiar 15x15");
        B.push_back("Rozmiar 25x25");
        B.push_back("Rozmiar 55x35");
        obcje.push_back(new  pozycja_w_menu(B));
        vector<string> C;
        C.push_back("Wyjdz");
        obcje.push_back(new  pozycja_w_menu(C));
    }

    void wyswietl_menu()
    {
        int ile=obcje.size();
        // system("clear"); poniżej "jakaś" szybsza alternatywa tej komendy
        cout << "\033[2J\033[1;1H";
        for(int i=0; i<ile; i++)
            cout <<  (i==kursor?">":" ")  << obcje[i]->tekst() <<endl;
    }

    void czytajAkcje()
    {
        init_keyboard();
        int klawisz;

        wyswietl_menu();
        while(trwa)
        {
            sleep(0.25);
            if(kbhit())
            {
                klawisz = readch();
                switch(klawisz)
                {
                case gora:
                {
                    kursor=(obcje.size()+kursor -1)%obcje.size();
                    wyswietl_menu();
                    cout << '\a';
                }
                break;
                case dol:
                {
                    kursor=(kursor +1)%obcje.size();

                    wyswietl_menu();
                    cout << '\a';
                }
                break;
                case akcja:
                {
                    obcje[kursor]->zmienStan();
                    ust->funkcje(kursor*100+obcje[kursor]->stan);
                    wyswietl_menu();
                    cout << '\a';
                }
                }
            }

        }
        close_keyboard();
    }


};


class ruch
{
public:
    ustawienia *gra;
    int klawisz;
    void rusz(int offsetx,int offsety )
    {
        int pietro=gra->poziom;
        int szerokosc=gra->wieklkoscmapyX;
        int wysokosc=gra->wieklkoscmapyY;
        int x=gra->plansza[pietro]->pozycja->x;
        int y =gra->plansza[pietro]->pozycja->y;
        if( gra->plansza[pietro]->tablica[offsetx+x][offsety+y] !=sciana&& !(y+offsety>=wysokosc|| y+offsety<0 ||x+offsetx>=szerokosc|| x+offsetx<0))
        {
            gra->plansza[pietro]->pozycja->x=x+offsetx;
            gra->plansza[pietro]->pozycja->y=y+offsety;
        }
    }

    void koniecgry()
    {
        int pietro=gra->poziom;
        int x=gra->plansza[pietro]->pozycja->x;
        int y =gra->plansza[pietro]->pozycja->y;
        if( gra->plansza[pietro]->tablica[x][y]==koniec)
        {
            // system("clear"); poniżej "jakaś" szybsza alternatywa tej komendy
            cout << "\033[2J\033[1;1H";
            cout << ">Wygrałeś"<<endl;;
            sleep(3);
            int temp;
            cin >> temp;

            gra->funkcje(400);
            klawisz=menu;
        }
    }

    void wspinaj()
    {
        int pietro=gra->poziom;
        int x=gra->plansza[pietro]->pozycja->x;
        int y =gra->plansza[pietro]->pozycja->y;

        if( gra->plansza[pietro]->tablica[x][y]==drabina_dol)
        {
            gra->poziom--;
            gra->plansza[pietro-1]->pozycja->x=gra->plansza[pietro]->pozycja->x;
            gra->plansza[pietro-1]->pozycja->y=gra->plansza[pietro]->pozycja->y;
        }

        if( gra->plansza[pietro]->tablica[x][y]==drabina_gora)
        {
            gra->poziom++;
            gra->plansza[pietro+1]->pozycja->x=gra->plansza[pietro]->pozycja->x;
            gra->plansza[pietro+1]->pozycja->y=gra->plansza[pietro]->pozycja->y;
        }


    }

    void czytajAkcje()
    {
        init_keyboard();

        gra->plansza[ gra->poziom]->rysuj();
        while(klawisz != menu)
        {
            sleep(0.25);
            if(kbhit())
            {
                klawisz = readch();
                switch(klawisz)
                {
                case gora:
                {
                    rusz(0,-1);
                    gra->plansza[ gra->poziom]->rysuj();
                }
                break;
                case dol:
                {
                    rusz(0,1);
                    gra->plansza[ gra->poziom]->rysuj();
                }
                break;
                case lewo:
                {
                    rusz(-1,0);
                    gra->plansza[ gra->poziom]->rysuj();
                }
                break;
                case prawo:
                {
                    rusz(1,0);
                    gra->plansza[ gra->poziom]->rysuj();
                }
                break;
                case akcja:
                {
                    wspinaj();
                    gra->plansza[ gra->poziom]->rysuj();
                    koniecgry();

                }
                }
            }

        }
        close_keyboard();
    }

    ruch(ustawienia* u)
    {
        gra=u;

    }

};

class generator_mapy
{
public:
    ustawienia *ust;
    int kretX;
    int kretY;
    int kretZ;
    vector<int> listaX;
    vector<int> listaY;
    vector<int> listaZ;
    generator_mapy(ustawienia *u)
    {
        ust=u;
    }

    bool nagore(){return    kretX-2>=0 ? ust->plansza[kretZ]->tablica[kretX-2][kretY]==sciana : 0;}                      //1
    bool nadol(){return     kretX+2<ust->wieklkoscmapyX ? ust->plansza[kretZ]->tablica[kretX+2][kretY]==sciana : 0;}    //2
    bool nalewo(){return    kretY-2>=0 ? ust->plansza[kretZ]->tablica[kretX][kretY-2]==sciana : 0;}                      //3
    bool naprawo(){return   kretY+2<ust->wieklkoscmapyY ? ust->plansza[kretZ]->tablica[kretX][kretY+2]==sciana : 0;}    //4
    bool schodynagore(){return kretZ+1<3 ? (ust->plansza[kretZ+1]->tablica[kretX][kretY]==sciana )&& (ust->plansza[kretZ]->tablica[kretX][kretY]==powietrze) : 0;}                   //5
    bool schodywdol(){return  kretZ-1>=0 ? (ust->plansza[kretZ-1]->tablica[kretX][kretY]==sciana  )&&  (ust->plansza[kretZ]->tablica[kretX][kretY]==powietrze): 0;}                    //6

    void Kopnagore(){ust->plansza[kretZ]->tablica[kretX-1][kretY]=powietrze;
                     ust->plansza[kretZ]->tablica[kretX-2][kretY]=powietrze;
                     kretX-=2;}                      //1
    void Kopnadol(){ust->plansza[kretZ]->tablica[kretX+1][kretY]=powietrze;
                     ust->plansza[kretZ]->tablica[kretX+2][kretY]=powietrze;
                     kretX+=2;}                      //2
    void Kopnalewo(){ust->plansza[kretZ]->tablica[kretX][kretY-1]=powietrze;
                     ust->plansza[kretZ]->tablica[kretX][kretY-2]=powietrze;
                     kretY-=2;}                      //3
    void Kopnaprawo(){ust->plansza[kretZ]->tablica[kretX][kretY+1]=powietrze;
                     ust->plansza[kretZ]->tablica[kretX][kretY+2]=powietrze;
                     kretY+=2;}                      //4
    void Kopschodynagore(){ust->plansza[kretZ]->tablica[kretX][kretY]=drabina_gora;
                     ust->plansza[kretZ+1]->tablica[kretX][kretY]=drabina_dol;
                     kretZ++;}                      //5
    void Kopschodywdol(){ust->plansza[kretZ]->tablica[kretX][kretY]=drabina_dol;
                     ust->plansza[kretZ-1]->tablica[kretX][kretY]=drabina_gora;
                     kretZ--;}                      //6
    int jestmozliwosc()
    {
        int jest=0;
        if(nagore())jest++;
        if(nadol())jest+=2;
        if(nalewo())jest+=4;
        if(naprawo())jest+=8;
        if(schodynagore())jest+=16;
        if(schodywdol())jest+=32;
        return jest;
    }

    void losuj_kop(int c)
    {int d=c;
        int e=0;
    while(d){if(d%2)e++;d=d/2;}
    int losowa=rand()%e +1;

    if(c%2==0)losowa++;
    c=c/2;
       if(losowa==1){if(rand()%2)Kopnagore();return;}
    losowa--;

    if(c%2==0)losowa++;
    c=c/2;
    if(losowa==1){Kopnadol();return;}
    losowa--;

     if(c%2==0)losowa++;
    c=c/2;
    if(losowa==1){if(rand()%2)Kopnalewo();return;}
    losowa--;

     if(c%2==0)losowa++;
    c=c/2;
    if(losowa==1){Kopnaprawo();return;}
    losowa--;

     if(c%2==0)losowa++;
    c=c/2;
    if(losowa==1){if(rand()%3)Kopschodynagore();return;}
    losowa--;

     if(c%2==0)losowa++;
    c=c/2;
    if(losowa==1){if(rand()%3)Kopschodywdol();return;}
    losowa--;



    }

    bool cofnij()
    {
    if(kretX==0&&kretY==0&&kretZ==1)return 0;
        kretX=listaX.back();
        listaX.pop_back();

        kretY=listaY.back();
        listaY.pop_back();

        kretZ=listaZ.back();
        listaZ.pop_back();
        return 1;
    }

    void wygeneruj()
    { bool meta=1;
        ust->poziom=1;
        ust->plansza[0]=new  wizja1(ust->wieklkoscmapyX,ust->wieklkoscmapyY);
        ust->plansza[1]=new  wizja2(ust->wieklkoscmapyX,ust->wieklkoscmapyY);
        ust->plansza[2]=new mapa(ust->wieklkoscmapyX,ust->wieklkoscmapyY);
        ust->plansza[0]->wstaw_gracza(new pozycja_gracza());
        ust->plansza[1]->wstaw_gracza(new pozycja_gracza());
        ust->plansza[2]->wstaw_gracza(new pozycja_gracza());
        bool kret=1;
        kretX=0;
        kretY=0;
        kretZ=1;
        ust->plansza[1]->tablica[0][0]=powietrze;

        while(meta)
        {
            if(kretX==ust->wieklkoscmapyX-1&&kretY==ust->wieklkoscmapyY-1){ ust->plansza[kretZ]->tablica[kretX][kretY]=koniec;
                                                                            meta=0;}
            else
            if( int c=jestmozliwosc())
               {listaX.push_back(kretX);
                listaY.push_back(kretY);
                listaZ.push_back(kretZ);
                losuj_kop(c);
               // ust->plansza[kretZ]->rysuj();
               }
            else
                {
                cofnij();

                }
        }
        int I=listaX.size();
        vector<int> lisX;
        vector<int> lisY;
        vector<int> lisZ;
        for(int i=0;i<I;i++)
            {   kret=1;
                kretX=listaX[i];
                kretY=listaY[i];
                kretZ=listaZ[i];
                while(kret)
                    {

                    int c=jestmozliwosc();
                    if(c)
                    {lisX.push_back(kretX);
                        lisY.push_back(kretY);
                    lisZ.push_back(kretZ);
                    losuj_kop(c);
                    //ust->plansza[kretZ]->rysuj();
                    }
                    else
                        {if(lisX.size()==0)kret=0;
                        else{
                            kretX=lisX.back();
                            lisX.pop_back();

                            kretY=lisY.back();
                            lisY.pop_back();

                            kretZ=lisZ.back();
                            lisZ.pop_back();
                            }
                        }
                    }

            }

    }

};

void ustawienia::funkcje(int jaka)
{
    switch(jaka)
    {
    case 1:
    {
        generator_mapy *gen=new generator_mapy(this);
        gen->wygeneruj();
        delete gen;
        ruszaj=new ruch(this);
        ruszaj->czytajAkcje();
        delete ruszaj;

    }
    break;
    case 0:
    {
        ruszaj=new ruch(this);
        ruszaj->czytajAkcje();
        delete ruszaj;
        mny->obcje[0]->stan=1;

    }
    break;
    case 100:
    {
        wieklkoscmapyX=15;
        wieklkoscmapyY=15;
        mny->obcje[0]->stan=0;
    }
    break;
    case 101:
    {
        wieklkoscmapyX=25;
        wieklkoscmapyY=25;
        mny->obcje[0]->stan=0;
    }
    break;
    case 102:
    {
        wieklkoscmapyX=55;
        wieklkoscmapyY=35;
        mny->obcje[0]->stan=0;
    }
    break;
    case 200:
    {
        mny->trwa=0;
    }
    break;
    case 400:
    {
        mny->obcje[0]->stan=0;
    }
    }


}


int main()
{

    srand( time( NULL ) );
    menugry *mmm= new menugry();
    mmm->czytajAkcje();
    cout << "Miłego Dnia" << endl;

    return 0;
}
