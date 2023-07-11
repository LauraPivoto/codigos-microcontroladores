#define LIGA (1 << PC0)
#define DESLIGA (1 << PC1)
#define S1 (1 << PB0)
#define S2 (1 << PB1)

#define AVANCO (1 << PD1)
#define RECUO (1 << PD2)
#define MOTOR (1 << PD3)

int main() {

  DDRD = (AVANCO + RECUO + MOTOR);
  PORTC = DESLIGA;

  PORTD &= ~(AVANCO + RECUO + MOTOR);

  //Configurar as interrupções no S1 (PB0) e S2 (PB1)
  PCICR = (1 << PCIE0);  //habilita interrupções no portal B
  PCMSK0 = (S1 + S2);    //habilita interrupções no PB0 e PB1

  sei();

  for (;;) {

    short int liga, desliga;
    liga = PINC & LIGA;
    desliga = PINC & DESLIGA;

    if (desliga == 0)
      PORTD &= ~(AVANCO + RECUO + MOTOR);
    else if (liga == LIGA)
      PORTD |= MOTOR;
  }
}

//Rotina de interrupções do portal B
ISR(PCINT0_vect) {

  if ((PINB & S1 == S1) && !(PORTD & RECUO))
    PORTD |= AVANCO;
  if (PINB & S2 == S2) {
    PORTD &= ~(MOTOR + AVANCO);
    PORTD |= RECUO;
  }
  if (PINB & S1 == 0)
    PORTD &= ~RECUO;
}