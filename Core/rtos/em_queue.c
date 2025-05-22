#include "em_queue.h"
#include "semphr.h"

ble_rw_t ble_rw_tdef;//定义结构体管理环形缓存区
SemaphoreHandle_t  xSemaphore = NULL;

void Queue_Init(void)
{
	Clean_ble_rw_t();
	xSemaphore = xSemaphoreCreateMutex();//创建互斥量
	if(xSemaphore==NULL)
	{
		printf("信号量创建失败\n");
	}
}

/**
  * @brief 清空接收缓存区
  * @param  
  * @retval 
  */
void Clean_ble_rw_t(void)
{
	ble_rw_tdef.r_index = 0;
	ble_rw_tdef.w_index = 0;
	ble_rw_tdef.left_line = 0;
	//memset(ble_rw_tdef.printer_buffer, 0, sizeof(ble_rw_tdef.printer_buffer));
}

/**
  * @brief 返回当前行数
  * @param  
  * @retval 
  */
uint32_t get_ble_rw_leftline(void)
{
    return ble_rw_tdef.left_line;
}


/**
  * @brief 写入一行数据
  * @param  
  * @retval 
  */
void Write_to_buffer(uint8_t *pdata,size_t length)
{
	//判断是否有更高优先级的任务需要调度
	static BaseType_t xHigherPriorityTaskWoken;
	if(length==0)
	{
		return;
	}
	if(ble_rw_tdef.left_line>=MAX_LINE) return;
	if(length>MAX_ONELINE_BYTE) length = MAX_ONELINE_BYTE;
	
	if(xSemaphoreTakeFromISR(xSemaphore,&xHigherPriorityTaskWoken)==pdPASS)
	{
		//这是 C 标准库中的内存复制函数，用于将一段内存的数据复制到另一段内存
    //目标内存地址，复制数据，长度
    memcpy(ble_rw_tdef.printer_buffer[ble_rw_tdef.w_index].buffer, pdata, length);
		ble_rw_tdef.w_index++;
		ble_rw_tdef.left_line++;
		if(ble_rw_tdef.w_index>=MAX_LINE){
			ble_rw_tdef.w_index = 0;//应该回到0
		}
		if(ble_rw_tdef.left_line>=MAX_LINE){
			ble_rw_tdef.left_line = MAX_LINE;
		}
		xSemaphoreGiveFromISR(xSemaphore,&xHigherPriorityTaskWoken);
	}
	// else {
    // 如果 xSemaphoreTakeFromISR 失败, 它会立即返回 pdFAIL,
    // 这种情况下，信号量没有被获取，因此也不需要释放。
    // 函数会直接跳到下面的 if(xHigherPriorityTaskWoken == pdTRUE)
    // }
	
	if(xHigherPriorityTaskWoken==pdTRUE)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

/**
  * @brief 读出一行数据
  * @param  
  * @retval 
  */
uint8_t* read_from_buffer(void)
{
	uint32_t index = 0;
	// 查看是否可以获得信号量，如果信号量不可用，则用10个时钟滴答来查看信号量是否可用
	if (xSemaphoreTake(xSemaphore, (portTickType)10) == pdPASS)
	{
		if(ble_rw_tdef.left_line>0)
		{
			ble_rw_tdef.left_line--;
			index = ble_rw_tdef.r_index;
			ble_rw_tdef.r_index++;
			if(ble_rw_tdef.r_index>=MAX_LINE){
				ble_rw_tdef.r_index=0;	//归0
			}
			xSemaphoreGive(xSemaphore);	
			return ble_rw_tdef.printer_buffer[index].buffer;
		}
		xSemaphoreGive(xSemaphore);	
		return NULL;
	}else
		printf("take fail\n");
		return NULL;
}

