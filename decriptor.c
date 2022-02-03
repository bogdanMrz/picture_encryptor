#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define lungime_header 54
typedef struct pixel pixel;
typedef union aleator aleator;
struct pixel
{
    unsigned char albastru;
    unsigned char verde;
    unsigned char rosu;

};

///PRIMA PARTE (CRIPTOGRAFIE)
union aleator
{
    unsigned nr;
    unsigned char bytes[3];
};
aleator *xorshift32(int lungime,unsigned r)
{
    aleator *nr_aleator=(aleator *)malloc((2*lungime-1)*sizeof(aleator));
    int i;
    for(i=0; i<2*lungime-1; i++)
    {
        r=r^r<<13;
        r=r^r>>17;
        r=r^r<<5;

        nr_aleator[i].nr=r;
    }
    return nr_aleator;
}



void citire_liniarizata(const char *cale,pixel **imagine,char **header,int *dim,int *lungime,int *inaltime,int *padding)
{
    FILE *fin=NULL;
    fin=fopen(cale,"rb");
    if(fin==NULL)
    {
        printf("deschidere gresita la citire %s",cale);
        return ;
    }


    fread(*header,sizeof(char),lungime_header,fin);

    fseek(fin,18,SEEK_SET);
    fread(lungime,sizeof(int),1,fin);
    fread(inaltime,sizeof(int),1,fin);

    *dim=(*lungime)*(*inaltime);
    *imagine=(pixel *)malloc((*dim)*sizeof(pixel));



    if(*lungime % 4 != 0)
        *padding = 4 - (3 * (*lungime)) % 4;
    else
        *padding = 0;

    int i;

    for(i=0; i<*inaltime; i++)
    {
        fseek(fin,-((*lungime)*3+(*padding))*(i+1),SEEK_END);
        fread((*imagine)+i*(*lungime),sizeof(pixel),*lungime,fin);
    }
    fclose(fin);

}


void scriere_liniarizata(const char *cale,char *header,pixel *imagine,int dim,int lungime,int inaltime,int padding)
{
    FILE *fout=NULL;
    fout=fopen(cale,"wb");
    if(fout==NULL)
    {
        printf("deschidere nereusita la scriere %s",cale);
        return ;
    }
    fwrite(header,sizeof(char),lungime_header,fout);
    unsigned char a=0;
    int i;
    for(i=0; i<inaltime; i++)
    {
        fwrite(imagine+dim-((i+1)*lungime),sizeof(pixel),lungime,fout);
        int k;
        for(k=1; k<=padding; k++)
            fwrite(&a,sizeof(char),1,fout);

    }



    fclose(fout);
}

//DECRIPTARE
int *genereaza_permutare_durstenfeld(aleator *nr,int lungime)
{

    int i,j;
    int *per;
    per=(int *)malloc(lungime*sizeof(int));
    for(i=0; i<lungime; i++)
        per[i]=i;
    int aux;
    int random;
    for(i=lungime-1,j=0; i>=1; i--,j++)
    {
        random=nr[j].nr%(i+1);
        aux=per[i];
        per[i]=per[random];
        per[random]=aux;

    }
    return per;
}



int *genereaza_permutare_inversa(int *per,int lungime)
{
    int i;
    int *inv;
    inv=(int *)malloc(lungime*sizeof(int));
    for(i=0; i<lungime; i++)
        inv[per[i]]=i;
    return inv;
}






void permuta(int *per,pixel *imagine,int lungime)
{
    pixel *aux;
    aux=(pixel *)malloc(lungime*sizeof(pixel));
    int i;
    for(i=0; i<lungime; i++)
    {
        aux[i].albastru=imagine[i].albastru;
        aux[i].rosu=imagine[i].rosu;
        aux[i].verde=imagine[i].verde;
    }
    for(i=0; i<lungime; i++)
    {
        imagine[per[i]].albastru=aux[i].albastru;
        imagine[per[i]].rosu=aux[i].rosu;
        imagine[per[i]].verde=aux[i].verde;
    }
    free(aux);
}






void xorare_criptare(pixel *imagine,aleator *secventa,int lungime,aleator SV)
{
    int i;


    imagine[0].albastru=SV.bytes[0] ^ imagine[0].albastru ^ secventa[lungime-1].bytes[0];
    imagine[0].verde=SV.bytes[1] ^ imagine[0].verde ^ secventa[lungime-1].bytes[1];
    imagine[0].rosu=SV.bytes[2] ^ imagine[0].rosu ^ secventa[lungime-1].bytes[2];

    for(i=1; i<lungime; i++)
    {

        imagine[i].albastru=imagine[i-1].albastru ^ imagine[i].albastru ^ secventa[i+lungime-1].bytes[0];
        imagine[i].verde=imagine[i-1].verde ^ imagine[i].verde ^ secventa[i+lungime-1].bytes[1];
        imagine[i].rosu=imagine[i-1].rosu ^ imagine[i].rosu ^ secventa[i+lungime-1].bytes[2];


    }

}


void xorare_decriptare(pixel *imagine,aleator *secventa,int lungime,aleator SV)
{
    int i;


    for(i=lungime-1; i>0; i--)
    {

        imagine[i].albastru=imagine[i-1].albastru ^ imagine[i].albastru ^ secventa[i+lungime-1].bytes[0];
        imagine[i].verde=imagine[i-1].verde ^ imagine[i].verde ^ secventa[i+lungime-1].bytes[1];
        imagine[i].rosu=imagine[i-1].rosu ^ imagine[i].rosu ^ secventa[i+lungime-1].bytes[2];

    }

    imagine[0].albastru=SV.bytes[0] ^ imagine[0].albastru ^ secventa[lungime-1].bytes[0];
    imagine[0].verde=SV.bytes[1] ^ imagine[0].verde ^ secventa[lungime-1].bytes[1];
    imagine[0].rosu=SV.bytes[2] ^ imagine[0].rosu ^ secventa[lungime-1].bytes[2];


}




void decriptare(const char *imagine_primita, const char *imagine_returnata,char *cheie)
{
    pixel *imagine=NULL;
    int lungime,inaltime,dim,padding;
    char *head=NULL;
    head=(char *)malloc(54*sizeof(char));
    citire_liniarizata(imagine_primita,&imagine,&head,&dim,&lungime,&inaltime,&padding);



    FILE *f=NULL;
    f=fopen(cheie,"rt");
    if(f==NULL)
    {
        printf("deschidere nereusita %s",cheie);
        return ;
    }
    unsigned r;
    aleator sv;
    fscanf(f,"%u%u",&r,&sv.nr);

    fclose(f);

    aleator *secventa=NULL;
    secventa=xorshift32(dim,r);
    if(secventa==NULL)
    {
        printf("alocare nereusita secventa aleatoare decriptare");
        return;
    }

    xorare_decriptare(imagine,secventa,dim,sv);



    int *permuare=genereaza_permutare_durstenfeld(secventa,dim);
    int *permuare_inversa=genereaza_permutare_inversa(permuare,dim);

    permuta(permuare_inversa,imagine,dim);


    scriere_liniarizata(imagine_returnata,head,imagine,dim,lungime,inaltime,padding);

    free(secventa);
    free(permuare);
    free(permuare_inversa);
    free(imagine);
    free(head);

}



///TEST XI PATRAT



double teoretic(int size)///f cu bara deasupra din formmula
{
    double aux=size-lungime_header;
    aux=aux/(256*3);
    return aux;
}



void alocare_RGB(double **rosu, double **verde,double **albastru,int size)
{
    *rosu=(double *)calloc(256,sizeof(double));
    *albastru=(double *)calloc(256,sizeof(double));
    *verde=(double *)calloc(256,sizeof(double));
}


void free_RGB(double **rosu, double **verde,double **albastru)
{
    free(*rosu);
    free(*verde);
    free(*albastru);
}


void vectori_RGB(FILE *imagine,int lungime,int inaltime,int padding,double *rosu,double *verde,double *albastru)
{

    int i,j;
    int c;
    for(i=0; i<inaltime; i++)
    {
        for(j=0; j<lungime; j++)
        {

            c=fgetc(imagine);
            albastru[c]++;




            c=fgetc(imagine);
            verde[c]++;



            c=fgetc(imagine);
            rosu[c]++;




        }
        fread(NULL,padding,1,imagine);
    }

}


double formula(double frecventa_teoretic,double fi)
{
    return ((fi-frecventa_teoretic)*(fi-frecventa_teoretic))/frecventa_teoretic;
}

double test_culoare(double frecventa_teoretic,double *culoare)
{
    int i;
    double fr=0;
    for(i=0; i<256; i++)
        fr=fr+formula(frecventa_teoretic,culoare[i]);
    return fr;
}

void frecv_test(const char *cale)
{
    FILE *imagine=NULL;
    imagine=fopen(cale,"rb");
    if(imagine==NULL)
    {
        printf("deschidere nereusita %s pentru XI patrat\n",cale);
        return ;
    }





    int lungime,inaltime,padding;
    fseek(imagine,18,SEEK_SET);
    fread(&lungime,sizeof(int),1,imagine);
    fread(&inaltime,sizeof(int),1,imagine);


    fseek(imagine,2,SEEK_SET);
    int size;
    fread(&size,sizeof(int),1,imagine);

    double frecventa_teoretic=teoretic(size);


    if(lungime % 4 != 0)
        padding = 4 - (3 * lungime) % 4;
    else
        padding = 0;




    double *rosu=NULL, *verde=NULL, *albastru=NULL;
    alocare_RGB(&rosu,&verde,&albastru,size);
    if(rosu==NULL)
    {
        printf("alocare nereusita rosu");
        return ;
    }
    if(verde==NULL)
    {
        printf("alocare nereusita verde");
        return ;
    }
    if(verde==NULL)
    {
        printf("alocare nereusita albastru");
        return ;
    }
    fseek(imagine,lungime_header,SEEK_SET);
    vectori_RGB(imagine,lungime,inaltime,padding,rosu,verde,albastru);
    printf("%s:\n",cale);

    printf("R: %.2f\n",test_culoare(frecventa_teoretic,rosu));
    printf("G: %.2f\n",test_culoare(frecventa_teoretic,verde));
    printf("B: %.2f\n",test_culoare(frecventa_teoretic,albastru));



    free_RGB(&rosu,&verde,&albastru);
    fclose(imagine);

}

int main(int argc, char const *argv[])
{
    char cheie[25] = "secret_key.txt";

    if (argc == 3)
    {
        decriptare(argv[1],argv[2],cheie);
        frecv_test(argv[1]);
    }
    else if(argc == 2)
    {
        char deciphered_file[strlen(argv[1])+10];
        strcpy(deciphered_file,"deciphered_");
        strcat(deciphered_file,argv[1]);
         
        decriptare(argv[1],deciphered_file,cheie);
        frecv_test(deciphered_file);
    }


    return 0;
}
