#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* --------------------------------- 中位值平均滤波 -------------------------------- */
/**
 * @brief 队列计数器
 */
uint8_t filterCounter = 0;
/**
 * @brief 原始数据队列数组
 */
#define FILTER_BUFFER_SIZE 32u
uint32_t filterBuffer[FILTER_BUFFER_SIZE];
/**
 * @brief 滤波后数据
 */
uint32_t filteredData;

/**
 * @brief std库快速排序所需的比较函数
 * @param a
 * @param b
 * @return
 */
int cmp(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}


/**
 * @brief 获取ADC原始值，将其填入待处理队列。队列满了以后对队列中数据进行快速排序
 * @param data ADC原始采样值
 * @return 队列是否已满
 */
uint8_t FilterDataAcquire(uint32_t data)
{
    if (filterCounter < FILTER_BUFFER_SIZE)
    {
        filterBuffer[filterCounter] = data;
        filterCounter++;
        return 0;
    }
    else
    {
        qsort(filterBuffer, FILTER_BUFFER_SIZE, sizeof(filterBuffer[0]), cmp);
        filterCounter = 0;
        return 1;
    }
}

/**
 * @brief 滤波核心程序，使用的是中位值平均滤波法。此算法有利于抑制脉冲性干扰，缺点在于占用RAM较大，计算时间较长。
 * @param originDataBuffer 原始数据队列
 * @param size 原始数据队列长度，用于算平均值
 * @return 滤波后结果
 */
uint32_t DataCompute(uint32_t *originDataBuffer, uint16_t size)
{
    uint32_t calcTemp = 0;
    double resultTemp = 0;
    for (uint32_t i = 2; i < size - 2; i++)
    {
        calcTemp += originDataBuffer[i];
    }
    resultTemp = calcTemp / ((size - 4) * 1.0);
    return (uint32_t)resultTemp;
}

/**
 * @brief ADC滤波主函数，实际使用中只需要调用这个即可
 * @param data 需要进入队列滤波的ADC原始值
 * @param result 用于返回的滤波后结果，通过传地址方式直接获取数据
 * @return 滤波是否完成，完成后可直接使用返回结果
 */
uint8_t Filter(uint32_t data, uint32_t *result)
{
    if (FilterDataAcquire(data))
    {
        filteredData = DataCompute(filterBuffer, FILTER_BUFFER_SIZE);
        *result = filteredData;
        return 1;
    }
    else
    {
        return 0;
    }
}




/* ----------------------------------- 卡尔曼 ---------------------------------- */
#define A 1 //系统增益，预测输出值前后的比值
// float x_k = 0;
// float x_k_1 = 0;
#define B 0 //控制增益，控制源
float u_k_1 = 0;    

#define Q 0.8 //越小表示越相信预测值，为0就是完全信任。越大表示越相信测量值
// float p_k = 0;
// float p_k_1 = 0;

#define H 1
#define R 0.2   //传感器靠谱程度
float k_k = 0;  
float z_k = 0;  //传感器数据，传入参数

float updata(float z_new)
{
    static float x_k = 0;   //卡尔曼滤波后得出的值，这里初值设为0
    static float x_k_1 = 0;
    static float p_k = 0;   //初值随便，会自己调整，不能设为0
    static float p_k_1 = 1;

    x_k = A * x_k_1 + B * u_k_1;
    p_k = p_k_1 + Q;
    k_k = p_k / (p_k + R);
    x_k = x_k + k_k * (z_new - x_k);
    p_k = (1 - k_k) * p_k;
    x_k_1 = x_k;
    p_k_1 = p_k;
    return x_k;
}

int main()
{
    int i;
    float temp[1000];
    float tempp = 32;
    // 设置随机种子
    srand(time(0));

    for (i = 1; i <= 1000; i++)
    {
        // 生成介于-0.4和0.4之间的随机数
        temp[i-1] = tempp+((float)rand() / RAND_MAX * 0.8 - 0.4);
        if(i%100 == 0)
            tempp += 1;
    }

    for (i = 0; i < 1000; i++)
    {
        printf("%.4f\r\n", updata(temp[i]));
    }

    return 0;
}