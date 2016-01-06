#ifndef _LLMNR_H
#define _LLMNR_H

void LlmnrInit(struct netif * net);
void LlmnrRun(void);
void LlmnrDown(void);

#endif