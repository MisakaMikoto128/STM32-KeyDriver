#include "Key.h"

int MATRIX_KEY_READ_PIN_NUM = MAX_READ_PIN_NUM;
int MATRIX_KEY_SET_PIN_NUM = MAX_SET_PIN_NUM;

/*
当前被扫描行的按键全部拉低
All keys of the currently scanned row are lowered level.
*/
unsigned char KeyPinRead(const KeyPin_t *keyinpin)
{
#if CHOOSE_KEW_ROW_LEVEL == 0
return !(keyinpin->GPIOx->IDR & keyinpin->GPIO_Pin);
#else
return (keyinpin->GPIOx->IDR & keyinpin->GPIO_Pin);
#endif
}

/**
 * @brief Makes the corresponding row of keys readable
 *
 * @param keyoutpin
 * @param value 1: make the corresponding row of keys readable and other row of keys unreadable
 */
void KeyPinSet(const KeyPin_t *keyoutpin, unsigned char value)
{
#if CHOOSE_KEW_ROW_LEVEL == 0
    if (!value)
        keyoutpin->GPIOx->BSRR = keyoutpin->GPIO_Pin;
    else
        keyoutpin->GPIOx->BRR = keyoutpin->GPIO_Pin;
#else
    if (!value)
        keyoutpin->GPIOx->BRR = keyoutpin->GPIO_Pin;
    else
        keyoutpin->GPIOx->BSRR = keyoutpin->GPIO_Pin;
#endif
}

KeyPin_t KeyInPins[MAX_READ_PIN_NUM] = {0};
KeyPin_t KeySetPins[MAX_SET_PIN_NUM] = {0};
//全部矩阵按键的当前状态
//The current state of all matrix keys
__IO KeyStateValue_t KeySta[MAX_READ_PIN_NUM][MAX_SET_PIN_NUM] = {0};
__IO int KeyPressTime[MAX_READ_PIN_NUM][MAX_SET_PIN_NUM] = {0};
__IO int KeyReleaseTime[MAX_READ_PIN_NUM][MAX_SET_PIN_NUM] = {0};
__IO unsigned char keybuf[MAX_READ_PIN_NUM][MAX_SET_PIN_NUM] = {0};

typedef struct _Key_t{
    unsigned char KeyID;
    unsigned char KeyState;
    unsigned char KeyPressTime;
    unsigned char KeyReleaseTime;
    unsigned char KeyBuf;
}Key_t;
/**
 * @brief 
 *
 * @param readPiNum  
 * @param setPinNum
 * @param keyinpins
 * @param keysetpins
 * @return true success
 * @return false faild
 */
bool KeyInit(int readPiNum, int setPinNum, const KeyPin_t *keyinpins, const KeyPin_t *keysetpins)
{
    if (readPiNum > MAX_READ_PIN_NUM || setPinNum > MAX_SET_PIN_NUM)
        return false;
    MATRIX_KEY_READ_PIN_NUM = readPiNum;
    MATRIX_KEY_SET_PIN_NUM = setPinNum;
    for (int i = 0; i < readPiNum; i++)
    {
        KeyInPins[i] = keyinpins[i];
    }
    for (int i = 0; i < setPinNum; i++)
    {
        KeySetPins[i] = keysetpins[i];
    }
    //初始化按键值位KEY_UP
    //Initialize the key value to KEY_UP
    memset((void *)KeySta, (int)KEY_UP, sizeof(KeySta)); 
    KeyGPIOConfig(readPiNum, setPinNum, keyinpins, keysetpins);
    return true;
}

void enableKey_GPIO_CLK(const KeyPin_t *keygpio)
{
    if (keygpio->GPIOx == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    #ifdef GPIOB
    else if (keygpio->GPIOx == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    #endif
    #ifdef GPIOC
    else if (keygpio->GPIOx == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    #endif
    #ifdef GPIOD
    else if (keygpio->GPIOx == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    #endif
    #ifdef GPIOE
    else if (keygpio->GPIOx == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    #endif
    #ifdef GPIOF
    else if (keygpio->GPIOx == GPIOF)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    #endif
    #ifdef GPIOG
    else if (keygpio->GPIOx == GPIOG)
    {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
    #endif
    #ifdef GPIOH
    else if (keygpio->GPIOx == GPIOH)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
    #endif
}
void KeyGPIOConfig(int readPiNum, int setPinNum, const KeyPin_t *keyinpins, const KeyPin_t *keysetpins)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    int i = 0;
    /* GPIO Ports Clock Enable */
    for (i = 0; i < readPiNum; i++)
    {
        enableKey_GPIO_CLK(&keyinpins[i]);
    }
    for (i = 0; i < setPinNum; i++)
    {
        enableKey_GPIO_CLK(&keysetpins[i]);
    }

    /*Configure input GPIO pin*/
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    for (i = 0; i < readPiNum; i++)
    {
        GPIO_InitStruct.Pin = keyinpins[i].GPIO_Pin;
        HAL_GPIO_Init(keyinpins[i].GPIOx, &GPIO_InitStruct);
    }

    /*Configure output GPIO pin*/
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    for (i = 0; i < setPinNum; i++)
    {
        GPIO_InitStruct.Pin = keysetpins[i].GPIO_Pin;
        HAL_GPIO_Init(keysetpins[i].GPIOx, &GPIO_InitStruct);
    }
}

/* 按键扫描函数，需在定时中断中调用，推荐调用间隔1ms */
/* Scanning key function, needs to be called in the timer interrupt, recommended to call every 1ms */
void KeyScan()
{
    unsigned char i;
    //矩阵按键扫描输出索引 
    //Matrix key scanning output index
    static unsigned char keyout = 0; 

    //将一行的4个按键值移入缓冲区
    //Move 4 keys in a row into the buffer
    for (i = 0; i < MATRIX_KEY_READ_PIN_NUM; i++)
    {
        keybuf[keyout][i] = (keybuf[keyout][i] << 1) | KeyPinRead(&KeyInPins[i]);
    }
    //消抖后更新按键状态
    //Update key status after debouncing
    //每行MATRIX_KEY_READ_PIN_NUM个按键，所以循环MATRIX_KEY_READ_PIN_NUM次
    //Loop MATRIX_KEY_READ_PIN_NUM times for each row
    for (i = 0; i < MATRIX_KEY_READ_PIN_NUM; i++) 
    {
        if ((keybuf[keyout][i] & 0x0F) == 0x00)
        { //连续4次扫描值为0，即4*4ms内都是按下状态时，可认为按键已稳定的弹起
            //Continuous 4 times scan value is 0, which means 4*4ms have been pressed, can be regarded as the key has been stable released
            KeyPressTime[keyout][i] = 0;
            if (KeyReleaseTime[keyout][i] == 0)
            {
                KeySta[keyout][i] = KEY_UP;
                INFO("keyup %d %d\r\n", keyout, i);
            }

            if (KeyReleaseTime[keyout][i] < KEY_DOUBLECLICK_TIME)
            {
                KeyReleaseTime[keyout][i]++;
            }
        }
        else if ((keybuf[keyout][i] & 0x0F) == 0x0F)
        { //连续4次扫描值为1，即4*4ms内都是弹起状态时，可认为按键已稳定的按下
            //Continuous 4 times scan value is 1, which means 4*4ms have been released, can be regarded as the key has been stable pressed

            //先检测是否连按
            //First check for double click
            if (KeyReleaseTime[keyout][i] < KEY_DOUBLECLICK_TIME && KeyReleaseTime[keyout][i] != 0)
            {
                KeySta[keyout][i] = KEY_DOUBLECLICK;
                INFO("keydoubleclick %d %d\r\n", keyout, i);
            }
            KeyReleaseTime[keyout][i] = 0;

            if (KeyPressTime[keyout][i] == 0)
            {
                KeySta[keyout][i] = KEY_DOWN;
                INFO("keydown %d %d\r\n", keyout, i);
            }

            if (KeyPressTime[keyout][i] >= KEY_LONG_PRESS_TIME)
            {
                KeySta[keyout][i] = KEY_LONG_PRESS;
                INFO("keylongpress %d %d\r\n", keyout, i);
            }
            else
            {
                KeyPressTime[keyout][i]++;
            }
        }
    }
    if (MATRIX_KEY_SET_PIN_NUM > 0)
    {
        //执行下一次的扫描输出
        //Execute the next scan output
        keyout++;                                       //输出索引递增 //Output index increases
        keyout = keyout & (MATRIX_KEY_SET_PIN_NUM - 1); //索引值加到4即归零 //Index value added to 4, which becomes 0
        for (i = 0; i < MATRIX_KEY_SET_PIN_NUM; i++)
        {
            KeyPinSet(&KeySetPins[i], (unsigned char)(i == keyout));
        }
    }
}
