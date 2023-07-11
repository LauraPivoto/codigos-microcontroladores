/*
Nomes: �lvaro L�cio Almeida Ribeiro, 
	   Edmundo Henrique de Paula Silva,
 	   Laura Pivoto Ambr�sio; 	   
Monitor: Ewel Fernandes Pereira;
 
 			Projeto E209
*/

#define DISTANCIA_TUBULACAO 5 // Dist�ncia tubula��o para detec��o de bolhas
#define pwm_saida (1 << PD6) // Motor Perist�ltico Rotativo
#define Alarme (1 << PD0) // Buzzer Alarme

#define alarme_ON (PORTD |= Alarme)
#define alarme_OFF (PORTD &= ~Alarme)

#define VALOR_UART ((msg_rx[0] - '0') * 100 + (msg_rx[1] - '0') * 10 + (msg_rx[2] - '0') * 1)

/* ---------------------------------- UART ---------------------------------- */
#define FOSC 16000000UL // Velocidade do Clock
#define BAUD 9600
#define MYUBRR FOSC / 16 / BAUD - 1

char msg_tx[20];
char msg_rx[32];
int pos_msg_rx = 0;
int tamanho_msg_rx = 3;
unsigned int volume = 0;
unsigned int tempo = 0;

/* ------------------------------- Outras vari�veis ------------------------------- */
int num_Gotas = 0;
float fluxoReal = 0.0f;
volatile unsigned long tempoAnterior = 0;  // Vari�vel para armazenar o tempo anterior da interrup��o
volatile unsigned long intervaloTempo = 0; // Vari�vel para armazenar o intervalo de tempo entre duas interrup��es

/* --------------------------- Declara��o das fun��es -------------------------- */
void UART_Init(unsigned int ubrr);
void UART_Transmit(char *dados);
void pararEAtivarAlarme();
void lerVolumeETempo();
float calcularFluxoReal();

int main()
{
    DDRD |= pwm_saida | Alarme; // configura sa�da para o PWM
    PORTD &= ~pwm_saida; // PWM inicia desligado
    PORTD |= (1 << PD2); // Ativa Pull-ups

    alarme_OFF;

    /* --------------------------- Configura��o do conversor AD -------------------------- */
	ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR); // Vref = 5V, resultado justificado � direita, pino de entrada ADC0
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (0 << ADATE) | (1 << ADIF) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC habilitado, in�cio da convers�o, modo de disparo livre desativado, ADC interrup��o flag, ADC interrup��o habilitada, prescaler 128

    /* Inicia a primeira convers�o ADC */
    ADCSRA |= (1 << ADSC);

    /* ----------------------------------- PWM ---------------------------------- */
    TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1); // Configura o modo FAST PWM e o modo do comparador A
    TCCR0B = 1; // Seleciona a op��o para frequ�ncia
    OCR0A = 0; // VALOR

    /* ------------------------------- Interrup��es ------------------------------- */
    EICRA |= (1 << ISC00) | (1 << ISC01); // INT0 ativa na borda de subida
    EIMSK |= (1 << INT0); // Liga o INT0
    sei(); // Ativa o servi�o de interrup��o

    /* ------------------------------ Leitura UART ------------------------------ */
    UART_Init(MYUBRR);
    lerVolumeETempo();

    while (1)
    {
        /* ------------------- Calcula a pot�ncia do motor e o erro ------------------ */
        fluxoReal = calcularFluxoReal();
        
        UART_Transmit("Fluxo real: ");
        itoa(fluxoReal, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("\n");
        
		float fluxoDefinido = volume / (tempo / 60.0);
        float potencia = fluxoDefinido / 450.0;
        OCR0A = potencia * 5; // Valor do PWM

        UART_Transmit("Fluxo Definido: ");
        itoa(fluxoDefinido, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("\n");

        UART_Transmit("Pot�ncia: ");
        itoa(potencia * 100.0, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("\n");

		UART_Transmit("PWM: ");
        itoa(OCR0A, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("V\n");
        
        float erro = ((fluxoReal - fluxoDefinido) / fluxoDefinido) * 1;

        UART_Transmit("Erro: ");
        itoa(erro, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("%\n");

        /* ------------------------------------ - ----------------------------------- */
        UART_Transmit("Gostaria de alterar os valores? 001-Sim  000-N�o\n");
        while (!(UCSR0A & (1 << RXC0)))
            ;
        DelayMS(100);
        unsigned int valor = VALOR_UART;

        if (valor == 1)
            lerVolumeETempo();
        /* ------------------------------------ - ----------------------------------- */
    }
}

ISR(INT0_vect)
{
    // Sensor de Gotas (PD2)
    num_Gotas++;

    UART_Transmit("Num Gotas: ");
    itoa(num_Gotas, msg_tx, 10);
    UART_Transmit(msg_tx);
    UART_Transmit("\n");
}

ISR(ADC_vect) // Rotina de Servi�o de Interrup��o para Convers�o ADC Completa
{
    float distancia = ((float)ADC / 1023.0) * 20.0; // Mapeia o valor do sensor para a faixa de dist�ncia

    if (distancia < DISTANCIA_TUBULACAO)
    {
        UART_Transmit("ALARME - DISTANCIA: ");
        itoa((int)distancia, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("cm\n");
        pararEAtivarAlarme(); // Bolha detectada, pare o equipamento e acione o alarme
    }

    // Inicia a pr�xima convers�o ADC
    ADCSRA |= (1 << ADSC);
}

/* ----------------------------- Fun��es UART ----------------------------- */
ISR(USART_RX_vect)
{
    // Escreve o valor recebido pela UART na posi��o pos_msg_rx do buffer msg_rx
    msg_rx[pos_msg_rx++] = UDR0;
    if (pos_msg_rx == tamanho_msg_rx)
        pos_msg_rx = 0;
}

void UART_Transmit(char *dados)
{
    // Envia todos os caracteres do buffer dados at� chegar um final de linha
    while (*dados != 0)
    {
        while (!(UCSR0A & (1 << UDRE0))); // Aguarda o fim da transmiss�o
        // Escreve o caractere no registro de transmiss�o
        UDR0 = *dados;
        // Passa para o pr�ximo caractere do buffer dados
        dados++;
    }
}

void UART_Init(unsigned int ubrr)
{
    // Configura a taxa de transmiss�o */
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char) ubrr;
    // Habilita a recep��o, transmiss�o e interrup��o na recep��o */
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    // Configura o formato da mensagem: 8 bits de dados e 1 bits de parada */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

/* ----------------------------- Outras fun��es ---------------------------- */
void pararEAtivarAlarme()
{
    alarme_ON;
    OCR0A = 0;
    PORTD &= ~pwm_saida;

    while (1)
    {
        // Loop indefinidamente para manter o alarme ligado
    }
}


float calcularFluxoReal()
{
	 // Calcular o fluxo real de gotas em ml/h usando a f�rmula fornecida
	float fluxoReal = ((num_Gotas * 20.0) / (tempo / 60.0)) * 0.05;
    return fluxoReal;
}

void lerVolumeETempo()
{
    while (1)
    {
        UART_Transmit("Insira o Volume:\n");
        while (!(UCSR0A & (1 << RXC0)))
            ;
        DelayMS(100);
        volume = VALOR_UART; // Volume lido em mL

        UART_Transmit("Volume: ");
        itoa(volume, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("ml\n");

        UART_Transmit("Insira o Tempo de Infus�o em minutos:\n");
        while (!(UCSR0A & (1 << RXC0)))
            ;
        DelayMS(100);
        tempo = VALOR_UART; // Tempo lido em min

        UART_Transmit("Tempo: ");
        itoa(tempo, msg_tx, 10);
        UART_Transmit(msg_tx);
        UART_Transmit("min\n");

        UART_Transmit("Confirma os valores? 001-Sim  000-N�o\n");
        while (!(UCSR0A & (1 << RXC0)))
            ;
        DelayMS(100);
        unsigned int valor = VALOR_UART;

        if (valor == 1)
            break;
    }
}

void DelayMS(unsigned int ms)
{
    // O timer 1 ser� usado com CTC mode e prescaler de 64.
    TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
    // Inicializando o valor de compara��o para uma interrup��o a cada milissegundo.
    OCR1A = 250;
    // Zera o timer.
    TCNT1 = 0;
    // Vari�vel para contar o n�mero de milissegundos passados.
    unsigned int counter = 0;
    // Enquanto o tempo desejado n�o for atingido.
    while (counter < ms)
    {
        // Verifica se o valor do timer atingiu o valor de compara��o.
        if (TIFR1 & (1 << OCF1A))
        {
            // Incrementa o contador de milissegundos.
            counter++;
            // Limpa a flag de compara��o de correspond�ncia.
            TIFR1 |= (1 << OCF1A);
        }
    }
    // Desliga o timer.
    TCCR1B = 0;
}