#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_spiffs.h"

#include "audio.h"
#include "config.h"
#include "sequencer.h"
#include "notes.h"
#include "voice.h"

#include "hardware.h"
#include "pax_gfx.h"
#include "pax_codecs.h"
#include "ili9341.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"

#define PERC_TILE_BASE 10
#define PERC_TILE_HEIGHT 10

#define NOTE_TILE_HEIGHT 5
#define NOTE_TILE_BASE 50

#define TEXT_BASE 200

#define PROGRESS_HEIGHT 5

#define NOTE_TRACK 3

#define MIN_BPM 90
#define MAX_BPM 200
#define BPM_DELTA 5

#define MIN_VOL 0.1f
#define MAX_VOL 2.0f
#define VOL_DELTA 0.1f

#define MIN_DETUNE -10
#define MAX_DETUNE 10
#define DETUNE_SCALE 0.05
#define DETUNE_ZERO -0.5

const float notes[] = {
  C4,H4,B4,A4,GIS3,G3,FIS3,F3,E3,DIS3,D3,CIS3,
  C3,H3,B3,A3,GIS2,G2,FIS2,F2,E2,DIS2,D2,CIS2,C2
};
#define NUM_NOTES (sizeof(notes) / sizeof(float))

typedef enum CursorArea_ {
  CURSOR_PERCUSSION,
  CURSOR_NOTES,
  CURSOR_BPM,
  CURSOR_VOLUME,
  CURSOR_DETUNE
} CursorArea;

extern float globalVolume;
static pax_buf_t screenBuf;
xQueueHandle buttonQueue;
extern Sequencer sequencer;
static int detune = 0;

CursorArea cursorArea = CURSOR_PERCUSSION;
int cursorX = 0;
int cursorY = 0;

void draw();
void drawProgress();
void handleKeyPress(int key);

extern "C" void app_main(void) {
  globalVolume = 0.8f;

  // Badge stuff
  bsp_init();
  bsp_rp2040_init();
  buttonQueue = get_rp2040()->queue;
  pax_buf_init(&screenBuf, NULL, 320, 240, PAX_BUF_16_565RGB);
  nvs_flash_init();

  //start audio
  audio_init();
  playSynth();

  draw();
  drawProgress();

  while(1) {
    rp2040_input_message_t message;
    if (xQueueReceive(buttonQueue, &message, 2)) {
      if (message.state) {
        handleKeyPress(message.input);
        draw();
      }        
    }
    drawProgress();    
  }
}

float nextVelocity(float vel) {
  if (vel < 0.33f) return 1.0f; //0 -> 1
  if (vel < 0.67f) return 0.0f;//0.33 -> 0
  if (vel < 1.0f) return 0.33f; //0.67 -> 0.33
  return 0.67f;
}

void changeVelocity(int trackIdx, int bar) {
  SequencerTrack *track = sequencer.getTrack(trackIdx);
  float velocity = track->velocity[bar];
  velocity = nextVelocity(velocity);
  track->velocity[bar] = velocity;
}

int noteIdxForPitch(float pitch) {
  for (int i = 0; i < NUM_NOTES; i++) {
    if (notes[i] == pitch) return i;
  }
  return -1;
}

void changeNote(int noteIdx, int bar) {
  SequencerTrack *track = sequencer.getTrack(NOTE_TRACK);
  int oldNoteIdx = noteIdxForPitch(track->pitch[bar]);
  bool sameNote = (noteIdx == oldNoteIdx);
  float velocity = sameNote ? nextVelocity(track->velocity[bar]) : 1.0f;
  track->velocity[bar] = velocity;
  track->pitch[bar] = notes[noteIdx];
}

void handleKeyPress(int key) {
  switch (key) {
    case RP2040_INPUT_JOYSTICK_LEFT:
      if ((cursorArea == CURSOR_PERCUSSION) || (cursorArea == CURSOR_NOTES)) {
        cursorX = (cursorX > 0) ? cursorX-1 : SEQUENCER_STEPS-1;
      } else if (cursorArea == CURSOR_BPM) {
        float bpm = sequencer.getBPM();
        bpm -= BPM_DELTA;
        if (bpm < MIN_BPM) bpm = MIN_BPM;
        sequencer.setBPM(bpm);
      } else if (cursorArea == CURSOR_VOLUME) {
        globalVolume-=VOL_DELTA;
        if (globalVolume < MIN_VOL) globalVolume = MIN_VOL;
      } else if (cursorArea == CURSOR_DETUNE) {
        if (detune > MIN_DETUNE) {
          detune--;
          sequencer.getVoice(NOTE_TRACK)->setTune(DETUNE_ZERO + DETUNE_SCALE * detune);
        }
      }
      break;
    case RP2040_INPUT_JOYSTICK_RIGHT:
      if ((cursorArea == CURSOR_PERCUSSION) || (cursorArea == CURSOR_NOTES)) {
        cursorX = (cursorX < SEQUENCER_STEPS-1) ? cursorX+1 : 0;
      } else if (cursorArea == CURSOR_BPM) {
        float bpm = sequencer.getBPM();
        bpm += BPM_DELTA;
        if (bpm > MAX_BPM) bpm = MAX_BPM;
        sequencer.setBPM(bpm);
      } else if (cursorArea == CURSOR_VOLUME) {
        globalVolume+=VOL_DELTA;
        if (globalVolume > MAX_VOL) globalVolume = MAX_VOL;
      } else if (cursorArea == CURSOR_DETUNE) {
        if (detune < MAX_DETUNE) {
          detune++;
          sequencer.getVoice(NOTE_TRACK)->setTune(DETUNE_ZERO + DETUNE_SCALE * detune);
        }
      }
      break;
    case RP2040_INPUT_JOYSTICK_DOWN:
      if (cursorArea == CURSOR_PERCUSSION) {
        if (cursorY < NOTE_TRACK-1) {
          cursorY += 1;
        } else {
          cursorArea = CURSOR_NOTES;
          cursorY = 0;
        }
      } else if (cursorArea == CURSOR_NOTES) {
        if (cursorY < NUM_NOTES-1) {
          cursorY += 1;
        } else {
          cursorArea = CURSOR_BPM;
        }
      } else if (cursorArea == CURSOR_BPM) {
        cursorArea = CURSOR_VOLUME;
      } else if (cursorArea == CURSOR_VOLUME) {
        cursorArea = CURSOR_DETUNE;
      }
      break;
    case RP2040_INPUT_JOYSTICK_UP:
      if (cursorArea == CURSOR_PERCUSSION) {
        if (cursorY > 0) {
          cursorY -= 1;
        }
      } else if (cursorArea == CURSOR_NOTES) {
        if (cursorY > 0) {
          cursorY -= 1;
        } else {
          cursorY = NOTE_TRACK-1;
          cursorArea = CURSOR_PERCUSSION;
        }
      } else if (cursorArea == CURSOR_BPM) {
        cursorArea = CURSOR_NOTES;
        cursorY = NUM_NOTES-1;
      } else if (cursorArea == CURSOR_VOLUME) {
        cursorArea = CURSOR_BPM;
      } else if (cursorArea == CURSOR_DETUNE) {
        cursorArea = CURSOR_VOLUME;
      }

      break;
    case RP2040_INPUT_BUTTON_ACCEPT:
      if (cursorArea == CURSOR_PERCUSSION) {
        changeVelocity(cursorY, cursorX);
      } else if (cursorArea == CURSOR_NOTES) {
        changeNote(cursorY, cursorX);
      }
      break;
    case RP2040_INPUT_BUTTON_HOME:
      REG_WRITE(RTC_CNTL_STORE0_REG, 0);
      esp_restart();
    default:
      break;
  }
}

void draw() {
  pax_col_t bgCol = pax_col_rgb(0,0,0);
  pax_col_t markerCol = pax_col_rgb(140,255,0);
  pax_simple_rect(&screenBuf, bgCol, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT-PROGRESS_HEIGHT);
  int slotWidth = ILI9341_WIDTH / SEQUENCER_STEPS;

  //draw percussion boxes
  for (int j=0; j<NUM_TRACKS-1; j++) {
    int y = PERC_TILE_BASE + j * PERC_TILE_HEIGHT;
    SequencerTrack *track = sequencer.getTrack(j);
    for (int i=0; i<SEQUENCER_STEPS; i++) {
      int x = i*slotWidth;
      int r = 100 + 155 * track->velocity[i];
      pax_col_t col = pax_col_rgb(r,100,100);
      pax_simple_rect(&screenBuf, col, x+1, y+1, slotWidth-2, PERC_TILE_HEIGHT-2);
      if ((cursorArea == CURSOR_PERCUSSION) && (j == cursorY) && (i == cursorX)) {
        pax_simple_line(&screenBuf, markerCol, x+1, y+1, x+slotWidth-2, y+1);
      }
    }
  }

  //draw notes boxes
  SequencerTrack *track = sequencer.getTrack(NOTE_TRACK);
  for (int i=0; i<SEQUENCER_STEPS; i++) {
    int x = i*slotWidth;
    int noteIdx = noteIdxForPitch(track->pitch[i]);
    float velocity = track->velocity[i];
    for (int j=0; j<NUM_NOTES; j++) {
      int y = NOTE_TILE_BASE + j * NOTE_TILE_HEIGHT;
      int r = 100 + 155 * (j == noteIdx ? velocity : 0);
      pax_col_t col = pax_col_rgb(r,100,100);
      pax_simple_rect(&screenBuf, col, x+1, y+1, slotWidth-2, NOTE_TILE_HEIGHT-2);
      if ((cursorArea == CURSOR_NOTES) && (j == cursorY) && (i == cursorX)) {
        pax_simple_line(&screenBuf, markerCol, x+1, y+1, x+slotWidth-2, y+1);
      }
    }
  }

  //draw text
  char str[20];
  int bpm = (int)(sequencer.getBPM()+0.5);
  snprintf(str,20,"BPM:%i",bpm);
  pax_col_t col = (cursorArea == CURSOR_BPM) ? markerCol : pax_col_rgb(150,150,150);
  pax_draw_text(&screenBuf, col, pax_font_saira_regular, pax_font_saira_regular->default_size, 10, TEXT_BASE, str);

  int volStep = (globalVolume + (VOL_DELTA / 2.0f)) / VOL_DELTA;
  int volume = (int)(volStep * 100 * VOL_DELTA);
  snprintf(str,20,"Vol:%i%%",volume);
  col = (cursorArea == CURSOR_VOLUME) ? markerCol : pax_col_rgb(150,150,150);
  pax_draw_text(&screenBuf, col, pax_font_saira_regular, pax_font_saira_regular->default_size, 120, TEXT_BASE, str);

  snprintf(str,20,"Tune:%i",detune);
  col = (cursorArea == CURSOR_DETUNE) ? markerCol : pax_col_rgb(150,150,150);
  pax_draw_text(&screenBuf, col, pax_font_saira_regular, pax_font_saira_regular->default_size, 240, TEXT_BASE, str);

  ili9341_write_partial_direct(get_ili9341(), screenBuf.buf_8bpp, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT-PROGRESS_HEIGHT);
}


void drawProgress() {
  int baseY = ILI9341_HEIGHT - PROGRESS_HEIGHT;
  int off = 2 * ILI9341_WIDTH * baseY;
  pax_col_t bgCol = pax_col_rgb(0,0,0);
  pax_col_t posCol = pax_col_rgb(255,100,0);
  pax_simple_rect(&screenBuf, bgCol, 0, baseY, ILI9341_WIDTH, PROGRESS_HEIGHT);
  int slotWidth = ILI9341_WIDTH / SEQUENCER_STEPS;
  int bar = sequencer.getCurrentBar();
  pax_simple_rect(&screenBuf, posCol, bar * slotWidth, baseY, slotWidth, PROGRESS_HEIGHT);

  ili9341_write_partial_direct(get_ili9341(), screenBuf.buf_8bpp+off, 0, ILI9341_HEIGHT-PROGRESS_HEIGHT, ILI9341_WIDTH, PROGRESS_HEIGHT);
}
