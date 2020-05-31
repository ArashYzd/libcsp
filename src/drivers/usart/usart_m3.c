/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2012 GomSpace ApS (http://www.gomspace.com)
Copyright (C) 2012 AAUSAT3 Project (http://aausat3.space.aau.dk)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "HL_sci.h"
#include "HL_sys_common.h"
#include "HL_system.h"
#include <csp/drivers/usart.h>
#include "FreeRTOS.h"
#include "os_semphr.h"
#include <stdio.h>

#include <csp/csp.h>
#include <csp/arch/csp_malloc.h>
#include <csp/arch/csp_thread.h>

typedef struct {
    csp_usart_callback_t rx_callback;
    void * user_data;
    sciBASE_t * fd;
    xTaskHandle rx_thread;
    uint32_t isListening;
} usart_context_t;

typedef struct {
    size_t length;
    uint8_t data[100];
} data_and_length;

data_and_length incomingData;
xSemaphoreHandle sciInturruptSem;

static int openPort(const char * device, sciBASE_t * return_fd) {

    return_fd = sciREG1;

    return CSP_ERR_NONE;
}

//static int configurePort(csp_usart_fd_t fd, const csp_usart_conf_t * conf) {
//
//    DCB portSettings = {0};
//
//
//static int setPortTimeouts(csp_usart_fd_t fd) {
//
//    COMMTIMEOUTS timeouts = {0};
//
//    if (!GetCommTimeouts(fd, &timeouts)) {
//        csp_log_error("Error gettings current timeout settings, error: %lu", GetLastError());
//        return CSP_ERR_INVAL;
//    }
//
//    timeouts.ReadIntervalTimeout = 5;
//    timeouts.ReadTotalTimeoutMultiplier = 1;
//    timeouts.ReadTotalTimeoutConstant = 5;
//    timeouts.WriteTotalTimeoutMultiplier = 1;
//    timeouts.WriteTotalTimeoutConstant = 5;
//
//    if(!SetCommTimeouts(fd, &timeouts)) {
//        csp_log_error("Error setting timeout settings, error: %lu", GetLastError());
//        return CSP_ERR_INVAL;
//    }
//
//    return CSP_ERR_NONE;
//}

void usart_rx_thread(void* params) {
    usart_context_t * ctx = (usart_context_t *) params;
    xSemaphoreTake(sciInturruptSem, 0);
    while (ctx->isListening) {
        xSemaphoreTake(sciInturruptSem, portMAX_DELAY);
        ctx->rx_callback(ctx->user_data, incomingData.data, incomingData.length, NULL);
    }
    return 0;
}

void sciNotification(sciBASE_t *sci, unsigned flags) {
    portBASE_TYPE higherTask = pdFALSE;
    xSemaphoreGiveFromISR(sciInturruptSem, &higherTask);
    sciReceive(sci, sizeof(data_and_length), (void *)&incomingData);
}

void esmGroup1Notification(int but) {
    return;
}

void esmGroup2Notification(int but) {
    return;
}

int csp_usart_write(sciBASE_t * fd, const void * data, size_t data_length) {

    data_and_length *out_data = csp_calloc(1, sizeof(*out_data));
    out_data->length = data_length;
    memcpy(&out_data->data, data, data_length);
    sciSend(fd, data_length, (void *) out_data);
    return (int) data_length;
}

int csp_usart_open(const csp_usart_conf_t *conf, csp_usart_callback_t rx_callback, void * user_data, sciBASE_t * return_fd) {
    _enable_IRQ();
    sciInit();
    sciBASE_t *fd;
    int res = openPort(conf->device, fd);
    if (res != CSP_ERR_NONE) {
        return res;
    }

//    res = configurePort(fd, conf);
//    if (res != CSP_ERR_NONE) {
//        CloseHandle(fd);
//        return res;
//    }

//    res = setPortTimeouts(fd);
//    if (res) {
//        CloseHandle(fd);
//        return res;
//    }

    usart_context_t * ctx = csp_calloc(1, sizeof(*ctx));
//    if (ctx == NULL) {
//        csp_log_error("%s: Error allocating context, device: [%s], errno: %s", __FUNCTION__, conf->device, strerror(errno));
//        CloseHandle(fd);
//        return CSP_ERR_NOMEM;
//    }
    ctx->rx_callback = rx_callback;
    ctx->user_data = user_data;
    ctx->fd = fd;
    ctx->isListening = 1;


    sciReceive(sciREG1, sizeof(data_and_length), (void *)&incomingData);
    vSemaphoreCreateBinary(sciInturruptSem);
    xTaskCreate(usart_rx_thread, "usart_rx", 50, (void *) ctx, 0, &ctx->rx_thread);

//    if (res) {
//        CloseHandle(ctx->fd);
//        csp_free(ctx);
//        return res;
//    }

    return CSP_ERR_NONE;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
