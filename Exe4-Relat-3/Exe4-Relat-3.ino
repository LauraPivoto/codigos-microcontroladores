//Elaborar um firmware, para manter um motor(PD6)ligado por 8 
//segundos sempre que o botão LIGA/NA (PB1 –Interrupção)for 
//pressionado. A velocidade desse motor será incrementada 
//em 12,5 % a cada segundo. É preciso prever o desligamento 
//através do botão DESLIGA/NF (PB2 –Interrupção)
//em qualquer momento da operação. 

//NF = PULL UP
//NA = PULL DOWN

//definição dos pinos
#define MOTOR (1<<PD6)
#define LIGA (1<<PB1)
#define DESLIGA (1<<PB2)

//definição das vars auxiliares
float DC = 0;
int cont = 0;
int segundos = 0;

int main (){
  
  //serial para testes
  Serial.begin(9600);
  
  //motor = saída e desligado
  DDRD = MOTOR;
  PORTD &= ~MOTOR;
  
  //NF É PULL UP E PRECISA DECLARAR
  PORTB = DESLIGA;
  
  //Configura as interrupções
  PCICR = (1<<PCIE0); //portal b
  PCMSK0= LIGA + DESLIGA; //define os pinos
  
  //para uso do temporizador
  //usando o prescaler de 64
  //tem que contar 250 vzs
  //isso da 1ms
  //se contar até 1000
  //1 seg
  TCCR2A = (0<<COM0A0) + (1<<WGM01) + (1<<WGM00);
  OCR2A = 249;
  TIMSK2 = (1<<OCIE2A);
  
  //pwm
  TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);
  TCCR0B = (1<<CS02) + (1<<CS00);  
  OCR0A = 0;  

  //interrupção global
  sei();

  for (;;){}
  
}


ISR(PCINT0_vect){
	if ((PINB & LIGA) == LIGA){
      	TCCR2B |= (1<<CS01) + (1<<CS00);
    }
  
  if(!(PINB & DESLIGA)){
    	TCCR2B = 0;
    	TCNT2 = 0;
    	Serial.println("BOTÃO DESLIGA PRESSIONADO");
    	DC = 0;
    	OCR0A = int(DC);
      //evitar gasto de energia e contagem de tempo
      PORTD &= ~MOTOR;
    }

}

//Função para quando o timer estourar
ISR(TIMER2_COMPA_vect){
  cont++;
  
  if(segundos == 8){
    segundos = 0;
    TCCR2B = 0;
    cont = 0;
    //evitar o tempo pequeno que o motor fica desligado.
    PORTD |= MOTOR;
  }
  
   if(cont == 1000) 
  {
    segundos++;
    Serial.println(segundos);
    cont=0;
    DC += (0.125*255);
    OCR0A = int(DC); 
    
  }
 
}

