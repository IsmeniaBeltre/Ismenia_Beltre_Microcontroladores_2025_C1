#include <stdio.h>
#include <stdlib.h>

// Definición de estados
typedef enum {
    ESTADO_INIT,
    ESTADO_CERRADO,
    ESTADO_ABIERTO,
    ESTADO_CERRANDO,
    ESTADO_ABRIENDO,
    ESTADO_DETENIDO,
    ESTADO_ERROR
} Estado;

// Definición de estructura para el control de la puerta
typedef struct {
    unsigned int LSC;
    unsigned int LSA;
    unsigned int SPP;
    unsigned int MA;
    unsigned int MC;
    unsigned int Cont_RT;
    unsigned int Led_A;
    unsigned int Led_C;
    unsigned int Led_ER;
    unsigned int COD_ERR;
    unsigned int DATOS_READY;
} ControlPuerta;

#define RT_MAX 180
#define ERROR_OK 0
#define ERROR_LS 1
#define ERROR_RT 2

// Variables globales
Estado estado_actual = ESTADO_INIT;
ControlPuerta puerta = {0};

// Prototipos de funciones
Estado Funcion_INIT();
Estado Funcion_ABIERTO();
Estado Funcion_ABRIENDO();
Estado Funcion_CERRADO();
Estado Funcion_CERRANDO();
Estado Funcion_DETENIDO();
Estado Funcion_ERROR();

// Función principal
int main() {
    while (1) {
        switch (estado_actual) {
            case ESTADO_INIT:
                estado_actual = Funcion_INIT();
                break;
            case ESTADO_ABIERTO:
                estado_actual = Funcion_ABIERTO();
                break;
            case ESTADO_ABRIENDO:
                estado_actual = Funcion_ABRIENDO();
                break;
            case ESTADO_CERRADO:
                estado_actual = Funcion_CERRADO();
                break;
            case ESTADO_CERRANDO:
                estado_actual = Funcion_CERRANDO();
                break;
            case ESTADO_DETENIDO:
                estado_actual = Funcion_DETENIDO();
                break;
            case ESTADO_ERROR:
                estado_actual = Funcion_ERROR();
                break;
        }
    }
    return 0;
}

Estado Funcion_INIT() {
    puerta.MA = puerta.MC = 0;
    puerta.Led_A = puerta.Led_C = puerta.Led_ER = 0;
    puerta.COD_ERR = ERROR_OK;
    puerta.Cont_RT = 0;
    puerta.DATOS_READY = 0;

    while (!puerta.DATOS_READY) {}

    if (puerta.LSC && !puerta.LSA) return ESTADO_CERRADO;
    if (!puerta.LSC && !puerta.LSA) return ESTADO_CERRANDO;
    if (puerta.LSC && puerta.LSA) {
        puerta.COD_ERR = ERROR_LS;
        return ESTADO_ERROR;
    }
    return ESTADO_INIT;
}

Estado Funcion_ABIERTO() {
    puerta.MA = 0;
    puerta.Led_A = 0;
    if (puerta.SPP) {
        puerta.SPP = 0;
        return ESTADO_CERRANDO;
    }
    return ESTADO_ABIERTO;
}

Estado Funcion_ABRIENDO() {
    puerta.MA = 1;
    puerta.Led_A = 1;
    puerta.Cont_RT = 0;

    while (1) {
        if (puerta.LSA) return ESTADO_ABIERTO;
        if (puerta.Cont_RT > RT_MAX) {
            puerta.COD_ERR = ERROR_RT;
            return ESTADO_ERROR;
        }
        if (puerta.SPP) return ESTADO_DETENIDO;
    }
}

Estado Funcion_CERRADO() {
    puerta.MC = 0;
    puerta.Led_C = 0;
    if (puerta.SPP) {
        puerta.SPP = 0;
        return ESTADO_ABRIENDO;
    }
    return ESTADO_CERRADO;
}

Estado Funcion_CERRANDO() {
    puerta.MC = 1;
    puerta.Led_C = 1;
    puerta.Cont_RT = 0;

    while (1) {
        if (puerta.LSC) return ESTADO_CERRADO;
        if (puerta.Cont_RT > RT_MAX) {
            puerta.COD_ERR = ERROR_RT;
            return ESTADO_ERROR;
        }
        if (puerta.SPP) return ESTADO_DETENIDO;
    }
}

Estado Funcion_DETENIDO() {
    puerta.MA = puerta.MC = 0;
    puerta.Led_A = puerta.Led_C = 0;
    while (!puerta.SPP) {}
    return (puerta.LSA) ? ESTADO_CERRANDO : ESTADO_ABRIENDO;
}

Estado Funcion_ERROR() {
    puerta.MA = puerta.MC = 0;
    puerta.Led_ER = 1;

    if (puerta.COD_ERR == ERROR_RT) {
        printf("\nERROR: REVISE EL SISTEMA. PRESIONE EL BOTÓN PARA CONTINUAR.\n");
    }

    while (!puerta.SPP) {}
    puerta.COD_ERR = ERROR_OK;
    return ESTADO_INIT;
}
