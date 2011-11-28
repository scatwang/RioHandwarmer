/*
 */

#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define F_CPU 1000000UL


#define discharge_turn_on(ch) {	PORTD |= ((ch) ? 0b10000000 : 0b01000000); TCCR1A |= ((ch) ? 0b00100000 : 0b10000000); }
#define discharge_turn_off(ch) {PORTD &= ((ch) ? 0b01111111 : 0b10111111); TCCR1A &= ((ch) ? 0b11011111 : 0b01111111); }
#define pwm(ch) (_SFR_MEM16(0x88+((ch)<<1)))

int GetADC(char port)
{
	ADMUX=0xc0|port;//0x40=VDD 0xc0=2.56

	ADCSRA|=(1<<ADSC);//Æô¶¯ADC×ª»»
	loop_until_bit_is_set(ADCSRA,ADIF);
	ADCSRA|=(1<<ADIF);
	int n=50;
	while(n--);//for a other adc loop
	return ADC;
}

int ledflag = 0;
void timerInterrupt()
{
    if(ledflag ++)
    {
        PORTD &= ~0b00100000;
        int led_ma = GetADC(6) * 3;
        if(led_ma<100)
            pwm(0) ++;
        else
            pwm(0) = 0;
    }
    else
        PORTD |= 0b00100000;
    ledflag = ledflag % 13;
}


int main(void)
{
	DDRB =  0b11111111; //PB0-SW PB1-PWM
	PORTB = 0b00000000;

	DDRD =  0b11110011; //PD2-CVCC PD3-Key
	PORTD = 0b00000000;

	TCCR0B = 3;          /* set prescaler to 1/64 */
	TCNT0 = -117;  // 128 Hz timer

	//pwm init
	TCCR1B  = 0B00000000 ;
	TCCR1A  = 0B00000010 ;
	TCCR1B  = 0B00011001 ;
	ICR1 = 8191;

	//adc init
	ADMUX=0xc0;//0x40=VCC 0xc0=2.56
	ADCSRA=0b10000111; //ADEN ADSC ADFR ADIF ADIE ADPS2:0

    TCCR1A |= 0b10000000;//pwm on

    while(1)
    {
        if(TIFR0 & (1 << TOV0)) // timer0 overflow
		{
			TIFR0 |= 1 << TOV0;  // clear pending flag
            TCNT0 = -117;  // 128 Hz timer
            timerInterrupt();
		}

    }

    return 0;
}
