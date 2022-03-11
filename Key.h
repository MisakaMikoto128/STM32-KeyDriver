#ifndef KEY_H_
#define KEY_H_

#include "main.h"
#include <stdbool.h>
#include <string.h>
#include "gpio.h"
#define MAX_READ_PIN_NUM 4
#define MAX_SET_PIN_NUM 4
typedef struct {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
}KeyPin_t;
#define KEY_STATE_NUM 4

typedef enum KEYID{
    KEY1 = 0,
    KEY2,
    KEY3,
    KEY4,
    KEY5,
    KEY6,
    KEY7,
    KEY8,
    KEY9,
    KEY10,
    KEY11,
    KEY12,
    KEY13,
    KEY14,
    KEY15,
    KEY16,
}KEYID_t;


typedef enum KeyStateValue{
    KEY_DOWN = 0, //shoule be 0,because the implementation of the function KeyiState()
    KEY_UP,
    KEY_LONG_PRESS,
    KEY_DOUBLECLICK,
}KeyStateValue_t;

typedef enum KeyState{
    KEY_NONE = -1,
    KEY1_Down = 0,
    KEY1_Up,
    KEY1_LongPress,
    KEY1_DoubleClick,
    KEY2_Down,
    KEY2_Up,
    KEY2_LongPress,
    KEY2_DoubleClick,
    KEY3_Down,
    KEY3_Up,
    KEY3_LongPress,
    KEY3_DoubleClick,
    KEY4_Down,
    KEY4_Up,
    KEY4_LongPress,
    KEY4_DoubleClick,
    KEY5_Down,
    KEY5_Up,
    KEY5_LongPress,
    KEY5_DoubleClick,
    KEY6_Down,
    KEY6_Up,
    KEY6_LongPress,
    KEY6_DoubleClick,
    KEY7_Down,
    KEY7_Up,
    KEY7_LongPress,
    KEY7_DoubleClick,
    KEY8_Down,
    KEY8_Up,
    KEY8_LongPress,
    KEY8_DoubleClick,
    KEY9_Down,
    KEY9_Up,
    KEY9_LongPress,
    KEY9_DoubleClick,
    KEY10_Down,
    KEY10_Up,
    KEY10_LongPress,
    KEY10_DoubleClick,
    KEY11_Down,
    KEY11_Up,
    KEY11_LongPress,
    KEY11_DoubleClick,
    KEY12_Down,
    KEY12_Up,
    KEY12_LongPress,
    KEY12_DoubleClick,
    KEY13_Down,
    KEY13_Up,
    KEY13_LongPress,
    KEY13_DoubleClick,
    KEY14_Down,
    KEY14_Up,
    KEY14_LongPress,
    KEY14_DoubleClick,
    KEY15_Down,
    KEY15_Up,
    KEY15_LongPress,
    KEY15_DoubleClick,
    KEY16_Down,
    KEY16_Up,
    KEY16_LongPress,
    KEY16_DoubleClick,
}KeyState_t;

#define KeyiState(key_id,state) (key_id*KEY_STATE_NUM+state)

#define KEY_LONG_PRESS_TIME 1000 /* unit:ms, hold for 1s and consider it a long press*/
#define KEY_DOUBLECLICK_TIME 100 /* unit:ms, press it twice for less than 0.1s, it is considered as double - click*/

#define CHOOSE_KEW_ROW_LEVEL 0


#define INFO(__fmt, ...) //printf("[%s:%d] "__fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
bool KeyInit(int readPiNum,int setPinNum,const KeyPin_t * keyinpins,const KeyPin_t * keysetpins);
void KeyGPIOConfig(int readPiNum, int setPinNum, const KeyPin_t *keyinpins, const KeyPin_t *keysetpins);
void KeyScan();



/* Key FIFO */
#define KEY_FIFO_SIZE	10
typedef struct
{
	KeyState_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */ /* Key value buffer */
	uint8_t Read;					/* 缓冲区读指针1 */ /* Buffer read pointer 1 */
	uint8_t Write;					/* 缓冲区写指针 */ /* Buffer write pointer */
}KEY_FIFO_T;
void Key_FIFO_Put(KeyState_t keystate);
void Key_FIFO_Clear(void);
KeyState_t Key_FIFO_Get(void);
bool isKeyFIFOEmpty(void);
inline void enable_key_up_envent(bool enable);
inline void disable_key_up_envent(void);
#endif /* KEY_H_ */

