/*
 * CIRCUIT:
 * 
 * Plug a 8 ohm speaker in the pins 3 and 11 from arduino.
 */
 
#include "musics.h"
#define bit_invert( field, pos ) field ^= (1 << pos);

unsigned int timer1 = 0;
unsigned int timer2 = 0;
unsigned int timer3 = 0;
unsigned int timer4 = 0;
unsigned int timer5 = 0;

uint8_t channels = 0;
uint16_t chNote[3] = {0,0,0};

uint8_t inline decode(uint8_t channels) {
  if (channels == 7) return 255;
  if (channels == 1 || channels == 2 || channels == 4) return 85;
  if (channels == 3 || channels == 5 || channels == 6) return 170;
  return 0;
}

void inline nonBlockingTones(unsigned int usec) {
  if (chNote[0] && usec - timer1 >= chNote[0]) {
      bit_invert(channels, 0);
      timer1 = usec;
      OCR2A = OCR2B = decode(channels);
  }

  if (chNote[1] && usec - timer2 >= chNote[1]) {
      bit_invert(channels, 1);
      timer2 = usec;
      OCR2A = OCR2B = decode(channels);
  }

  if (chNote[2] && usec - timer3 >= chNote[2]) {
      bit_invert(channels, 2);
      timer3 = usec;
      OCR2A = OCR2B = decode(channels);
  }
}

// Posição geral dentro da música MIDI
uint32_t midiPosition = 0;

// Estado de cada trilha
struct trackState {
  unsigned int pos = 0;
  uint32_t nextEvent = 0;
} track[3];

void resetPlayerStatus() {
  for(uint8_t i=0; i<3;i++) {
    track[i].pos = 0;
    track[i].nextEvent = 0;
  }
  midiPosition = 0;
}

void inline nonBlockingMusicPlayer(unsigned int usec, struct music music) {
  if(usec-timer4>=music.bpm) {
    midiPosition++;
    timer4 = usec;
  }
  if(midiPosition == track[0].nextEvent) {
      chNote[0] = pgm_read_word(music.notes[0] + track[0].pos);
      track[0].pos++;
      track[0].nextEvent = pgm_read_dword(music.tempo[0] + track[0].pos);
  }
  if(midiPosition == track[1].nextEvent) {
      chNote[1] = pgm_read_word(music.notes[1] + track[1].pos);
      track[1].pos++;
      track[1].nextEvent = pgm_read_dword(music.tempo[1] + track[1].pos);
  }
  if(midiPosition == track[2].nextEvent) {
      chNote[2] = pgm_read_word(music.notes[2] + track[2].pos);
      track[2].pos++;
      track[2].nextEvent = pgm_read_dword(music.tempo[2] + track[2].pos);
  }
}

uint8_t inline isEndOfMusic(unsigned int msec, struct music music) {
  if(msec - timer5 >= 1000) {
    if(midiPosition >= music.trackSize[0] && midiPosition >= music.trackSize[1] && midiPosition >= music.trackSize[2]) {
      return 1;
    }
    timer5 = msec;
  }
  return 0;
}

void setup() {
  pinMode(3,OUTPUT);
  pinMode(11, OUTPUT);
  TCCR2A = _BV(COM2A1) | _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); 
  TCCR2B = _BV(CS20);
  OCR2A = OCR2B = 0;
}

uint8_t idx = 0;
struct music * playList[] = {
    //&axelf,
    &castlevania,
    //&mario_battle,
    &mario_theme,
    //&mario_underworld,
    //&mario_castle,
    //&mario_flag,
    //&mario_underwater
  };
  
void loop() {
  unsigned int usec = micros();
  unsigned int msec = millis();
  nonBlockingTones(usec);
  nonBlockingMusicPlayer(usec, *playList[idx]);
  
  if(isEndOfMusic(msec, *playList[idx])) {
      resetPlayerStatus();
      idx++;
      if(idx>=2)idx=0;
  }
  
}
