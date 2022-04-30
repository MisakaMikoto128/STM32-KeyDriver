#include "Key.h"

int MATRIX_KEY_READ_PIN_NUM = MAX_READ_PIN_NUM;
int MATRIX_KEY_SET_PIN_NUM = MAX_SET_PIN_NUM;
bool enable_key_up_envent_flag = false;
bool key_gpio_configed = false;

#define GPIO_NUMBER (16U)
/*

--  --  --  -- Read0
0|  1|  2|  3|
--  --  --  -- Read1
4|  5|  6|  7|
--  --  --  -- Read2
8|  9| 10| 11|
--  --  --  -- Read3 MATRIX_KEY_READ_PIN_NUM
 |   |   | 16|
Set0 Set1 Set2 Set3
MATRIX_KEY_SET_PIN_NUM
key gpio init should be init before keyscan.
*/
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
        keyoutpin->GPIOx->BSRR = (uint32_t)keyoutpin->GPIO_Pin << GPIO_NUMBER;
#else
    if (!value)
        keyoutpin->GPIOx->BSRR = (uint32_t)keyoutpin->GPIO_Pin << GPIO_NUMBER;
    else
        keyoutpin->GPIOx->BSRR = keyoutpin->GPIO_Pin;
#endif
}

KeyPin_t KeyInPins[MAX_READ_PIN_NUM] = {0};
KeyPin_t KeySetPins[MAX_SET_PIN_NUM] = {0};

typedef struct _Key_t
{
    // current state of key
    KeyStateValue_t State;
    int PressTime;
    int ReleaseTime;
    unsigned char KeyBuf;
} Key_t;

__IO Key_t KeyMat[MAX_SET_PIN_NUM][MAX_READ_PIN_NUM];

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
    MATRIX_KEY_SET_PIN_NUM = setPinNum == 0 ? 1 : setPinNum;
    for (int i = 0; i < readPiNum; i++)
    {
        KeyInPins[i] = keyinpins[i];
    }
    for (int i = 0; i < setPinNum; i++)
    {
        KeySetPins[i] = keysetpins[i];
    }
    // Initialize the key value to KEY_UP
    Key_t *pKeyMat = (Key_t *)&KeyMat;
    for (int i = 0; i < MAX_READ_PIN_NUM * MAX_SET_PIN_NUM; i++)
    {
        pKeyMat[i].State = KEY_UP;
        pKeyMat[i].PressTime = 0;
        pKeyMat[i].ReleaseTime = 0;
        pKeyMat[i].KeyBuf = 0;
    }
    KeyGPIOConfig(readPiNum, setPinNum, keyinpins, keysetpins);
    set_key_gpio_configed(true);
    return get_key_gpio_configed();
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
    // Matrix key scanning output index
    static unsigned char keyout = 0;

    // sure the key gpio is configed before scan
    if (get_key_gpio_configed() == false)
        return;

    // Move MATRIX_KEY_READ_PIN_NUM keys in a row into the buffer
    for (i = 0; i < MATRIX_KEY_READ_PIN_NUM; i++)
    {
        KeyMat[keyout][i].KeyBuf = (KeyMat[keyout][i].KeyBuf << 1) | KeyPinRead(&KeyInPins[i]);
    }

    // Update key status after debouncing
    //每行MATRIX_KEY_READ_PIN_NUM个按键，所以循环MATRIX_KEY_READ_PIN_NUM次
    // Loop MATRIX_KEY_READ_PIN_NUM times for each row
    for (i = 0; i < MATRIX_KEY_READ_PIN_NUM; i++)
    {
        if ((KeyMat[keyout][i].KeyBuf & KEY_FILTER_TIMES) == 0x00)
        { //连续4次扫描值为0，即4*4ms内都是按下状态时，可认为按键已稳定的弹起
            // Continuous 4 times scan value is 0, which means 4*4ms have been pressed, can be regarded as the key has been stable released
            KeyMat[keyout][i].PressTime = 0;

            if (KeyMat[keyout][i].ReleaseTime == 0)
            {
                KeyMat[keyout][i].State = KEY_UP;
                INFO("keyup %d %d\r\n", keyout, i);
                if (enable_key_up_envent_flag)
                {
                    Key_FIFO_Put(KeyiState(i * MATRIX_KEY_SET_PIN_NUM + keyout, KeyMat[keyout][i].State));
                }
            }

            if (KeyMat[keyout][i].ReleaseTime < KEY_DOUBLECLICK_TIME)
            {
                KeyMat[keyout][i].ReleaseTime++;
            }
        }
        else if ((KeyMat[keyout][i].KeyBuf & KEY_FILTER_TIMES) == KEY_FILTER_TIMES)
        { //连续4次扫描值为1，即4*4ms内都是弹起状态时，可认为按键已稳定的按下
            // Continuous 4 times scan value is 1, which means 4*4ms have been released, can be regarded as the key has been stable pressed

            //先检测是否连按
            // First check for double click
            if (KeyMat[keyout][i].ReleaseTime < KEY_DOUBLECLICK_TIME && KeyMat[keyout][i].ReleaseTime != 0)
            {
                KeyMat[keyout][i].State = KEY_DOUBLECLICK;
                INFO("keydoubleclick %d %d\r\n", keyout, i);
                Key_FIFO_Put(KeyiState(i * MATRIX_KEY_SET_PIN_NUM + keyout, KeyMat[keyout][i].State));
            }
            else
            {
                if (KeyMat[keyout][i].PressTime == 0)
                {
                    KeyMat[keyout][i].State = KEY_DOWN;
                    INFO("keydown %d %d\r\n", keyout, i);
                    Key_FIFO_Put(KeyiState(i * MATRIX_KEY_SET_PIN_NUM + keyout, KeyMat[keyout][i].State));
                }
            }
            KeyMat[keyout][i].ReleaseTime = 0;

            if (KeyMat[keyout][i].PressTime < KEY_LONG_PRESS_TIME)
            {
                KeyMat[keyout][i].PressTime++;
            }
            else if (KeyMat[keyout][i].PressTime == KEY_LONG_PRESS_TIME)
            {
                KeyMat[keyout][i].PressTime = KEY_LONG_PRESS_CONTINUE_TIME; // avoid repeat trigger
                KeyMat[keyout][i].State = KEY_LONG_PRESS;
                INFO("keylongpress %d %d\r\n", keyout, i);
                Key_FIFO_Put(KeyiState(i * MATRIX_KEY_SET_PIN_NUM + keyout, KeyMat[keyout][i].State));
            }
        }
    }
    if (MATRIX_KEY_SET_PIN_NUM > 1) // Scan only if there are more than one line
    {
        // Execute the next scan output
        keyout++; //输出索引递增 //Output index increases
        if (keyout > (MATRIX_KEY_SET_PIN_NUM - 1))
        { // Index value added to MATRIX_KEY_SET_PIN_NUM - 1, which becomes 0
            keyout = 0;
        }
        for (i = 0; i < MATRIX_KEY_SET_PIN_NUM; i++)
        {
            KeyPinSet(&KeySetPins[i], (unsigned char)(i == keyout));
        }
    }
}

/*  Key FIFO */
static KEY_FIFO_T s_tKey; /* Key FIFO */
/**
 * @brief put a key state value to the key fifo
 *
 * @param keystate key state value
 */
void Key_FIFO_Put(KeyState_t keystate)
{
    s_tKey.Buf[s_tKey.Write] = keystate;

    if (++s_tKey.Write >= KEY_FIFO_SIZE)
    {
        s_tKey.Write = 0;
    }
}
/**
 * @brief clear the key fifo
 *
 */
void Key_FIFO_Clear(void)
{
    s_tKey.Read = s_tKey.Write;
}
/**
 * @brief read and pop a key state value from the key fifo
 *
 * @return KeyState_t key state value
 */
KeyState_t Key_FIFO_Get(void)
{
    KeyState_t ret = KEY_NONE;

    if (s_tKey.Read != s_tKey.Write)
    {
        ret = s_tKey.Buf[s_tKey.Read];

        if (++s_tKey.Read >= KEY_FIFO_SIZE)
        {
            s_tKey.Read = 0;
        }
    }
    return ret;
}

bool isKeyFIFOEmpty(void)
{
    return (s_tKey.Read == s_tKey.Write);
}

inline void enable_key_up_envent(bool enable)
{
    enable_key_up_envent_flag = enable;
}

inline void disable_key_up_envent(void)
{
    enable_key_up_envent_flag = false;
}

inline void set_key_gpio_configed(bool flag)
{
    key_gpio_configed = flag;
}

inline bool get_key_gpio_configed()
{
    return key_gpio_configed;
}
