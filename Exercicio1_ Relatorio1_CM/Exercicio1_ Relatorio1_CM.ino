//Declaração de bibliotecas

//Declaração de constantes
#define CHAVE (1<<PD3)
#define DESLIGA (1<<PC1)
#define LIGA (1<<PC0)
#define BAIXO (1<<PD4)
#define ALTO (1<<PD5)
#define BOMBA (1<<PB0)

//Declaração de variáveis globais

//Declaração de funções

int main(){

  //Direcionamento das Portas (DDRx) - Modificar só os pinos de saída
  DDRB = BOMBA; //PB0 como saída (Bomba)
  DDRC = 0; //Todos os pinos do portal são entradas
  DDRD = 0; //Todos os pinos do portal são entradas

  //Habilita o circuito de PULL-UP nos botões NFs (PORTx)
  PORTC |= DESLIGA; //PC1 com circuito de pull-up habilitado

  //Estabelecer a condição inicial das saídas
  PORTB &= ~BOMBA; //Bomba desligada
  
  for(;;){ //Loop Infinito

    //Leitura da Chave (PD3)
    short int chave;
    chave = PIND & CHAVE;

    if(chave == CHAVE){ //Se chave estiver no modo manual

      //Leitura dos botões (Desliga e Liga)
      short int liga, desliga;
      desliga = PINC & DESLIGA;
      liga = PINC & LIGA;

      if(!(desliga == DESLIGA)) //Botão Desliga pressionado
        PORTB &= ~BOMBA; //Desligar a Bomba

      else if (liga == 0b00000001) //Botão Liga pressionado
        PORTB |= BOMBA; //Ligar a Bomba
    }
    else{ //Chave no modo Automático

      //Leitura dos Sensores (Baixo e Alto)
      short int alto, baixo;
      baixo = PIND & BAIXO;
      alto = PIND & ALTO;

      if((baixo == BAIXO) && (alto == ALTO)) //Ambos acionados
        PORTB &= ~BOMBA; //Desligar a bomba
      else if((!(baixo == BAIXO)) && (!(alto == alto))) //Ambos desacionados
        PORTB |= BOMBA; //Ligar a bomba
    }

  }

}