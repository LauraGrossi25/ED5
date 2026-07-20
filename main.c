
//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"



#define IREF_AMP  12.85f
#define VP        311.13f

#define KP       69.282f
#define KI       161116.28f
#define K1       0.0000250f
#define K2       1.99964f
#define INV_VDC  0.0025f   //   ---> 1/400 

float ig = 0.0f;
float vg = 0.0f;
float iref = 0.0f;

float erro = 0.0f;
float s = 0.0f;
float u = 0.0f;

float e1 = 0.0f;
float e2 = 0.0f;
float s1 = 0.0f;
float s2 = 0.0f;

float modulante = 0.0f;

//uint32_t contadorAmostras = 0;


/*================================================*/
/* Geração da referência */
/*================================================*/

  
void geraReferencia(void)
{
    iref = IREF_AMP * (vg / VP);
    
}

//TESTE 1
/*
void geraReferencia(void)
{
    float iref_amp;

    if(contadorAmostras < 83U)
    {
        iref_amp = 12.85f;
    }
    else
    {
        iref_amp = 6.42f;
    }

    iref = iref_amp * (vg / VP);

    contadorAmostras++;
}



/*================================================*/
/* Controlador PR                                 */
/*================================================*/

void controladorPR(void)
{
    // Erro de corrente 

    erro = iref - ig;

    // Parte ressonante 

    s = K1 * erro - K1 * e2 + K2 * s1 - s2;

    // Atualização dos estados 

    e2 = e1;
    e1 = erro;

    s2 = s1;
    s1 = s;

    //Controlador 

    u = KP * erro + KI * s;

    // Feedforward 

    u += vg;

    //Normalização 

    modulante = u * INV_VDC;

    // Saturação 

    if(modulante > 1.0f)
    {
        modulante = 1.0f;
    }

    if(modulante < -1.0f)
    {
        modulante = -1.0f;
    }
}

/*================================================*/
/* Atualização dos PWMs                           */
/*================================================*/

void atualizaPWM(void)
{
    float duty1;
    float duty2;

    uint16_t cmpa1;
    uint16_t cmpa2;

    /* Conversão da modulante [-1, 1] para duty [0, 1] */
    duty1 = 0.5f * (modulante + 1.0f);
    duty2 = 0.5f * (-modulante + 1.0f);

    /* Conversão do duty para CMPA */
    cmpa1 = (uint16_t)(duty1 * 2500.0f);
    cmpa2 = (uint16_t)(duty2 * 2500.0f);

    /* Atualização dos comparadores */
    EPWM_setCounterCompareValue(myEPWM0_BASE,
                                EPWM_COUNTER_COMPARE_A,
                                cmpa1);

    EPWM_setCounterCompareValue(myEPWM1_BASE,
                                EPWM_COUNTER_COMPARE_A,
                                cmpa2);
}

__interrupt void INT_myADC1_1_ISR(void)
{
    float vadc_ig;
    float vadc_vg;

    /* Leitura do ADC de corrente */
    vadc_ig = ((float)ADC_readResult(ADCARESULT_BASE,
                                     ADC_SOC_NUMBER0)
                                     * 3.0f / 4095.0f);

    ig = 10.0f * vadc_ig - 15.0f;


    /* Leitura do ADC de tensão */
    vadc_vg = ((float)ADC_readResult(ADCBRESULT_BASE,
                                     ADC_SOC_NUMBER0)
                                     * 3.0f / 4095.0f);

    vg = 210.0f * vadc_vg - 315.0f;


    /* Geração da referência */
    geraReferencia();


    /* Controlador PR */
    controladorPR();


    /* Atualização dos PWMs */
    atualizaPWM();


    /* Limpa flag da interrupção do ADCB */
    ADC_clearInterruptStatus(ADCB_BASE,
                             ADC_INT_NUMBER1);

    /* Libera grupo de interrupção */
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/*================================================*/
/* Programa principal */
/*================================================*/

void main(void)
{
    Device_init();

    Interrupt_initModule();

    Interrupt_initVectorTable();

    Board_init();

    EINT;
    ERTM;

    while(1)
    {
  
    }
}


