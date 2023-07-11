//Declaração de bibliotecas

//Declaração de constantes

//Declaração de variáveis globais

//Declaração de funções

int main(){

  //Direcionamento das Portas (DDRx) - Modificar só os pinos de saída
  DDRB = 0b00000001; //PB0 como saída (Bomba)
  DDRC = 0; //Todos os pinos do portal são entradas
  DDRD = 0; //Todos os pinos do portal são entradas

  //Habilita o circuito de PULL-UP nos botões NFs (PORTx)
  PORTC = 0b00000010; //PC1 com circuito de pull-up habilitado

  //Estabelecer a condição inicial das saídas
  PORTB = 0b00000000; //Bomba desligada
  
  for(;;){ //Loop Infinito

    //Leitura da Chave (PD3)
    short int chave;
    chave = PIND & 0b00001000;

    if(chave == 0b00001000){ //Se chave estiver no modo manual

      //Leitura dos botões (Desliga e Liga)
      short int liga, desliga;
      desliga = PINC & 0b00000010;
      liga = PINC & 0b00000001;

      if(desliga == 0b00000000) //Botão Desliga pressionado
        PORTB = 0b00000000; //Desligar a Bomba

      else if (liga == 0b00000001) //Botão Liga pressionado
        PORTB = 0b00000001; //Ligar a Bomba
    }
    else{ //Chave no modo Automático

      //Leitura dos Sensores (Baixo e Alto)
      short int alto, baixo;
      baixo = PIND & 0b00010000;
      alto = PIND & 0b00100000;

      if((baixo == 0b00010000) && (alto == 0b00100000)) //Ambos acionados
        PORTB = 0b00000000; //Desligar a bomba
      else if((baixo == 0) && (alto == 0)) //Ambos desacionados
        PORTB = 0b00000001; //Ligar a bomba
    }

  }

}