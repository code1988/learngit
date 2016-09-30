
#include "base.h"
#include "ring.h"
#include "rpp_to.h"
#include "machine.h"

static int _rpp_ring_init_machine (STATE_MACH_T* this)
{
	this->state = BEGIN;
	(*(this->enter_state)) (this);
	return 0;
}

static int _rpp_ring_iterate_machines (rppRing_t *this, int (*iter_callb)(STATE_MACH_T*), Bool exit_on_non_zero_ret)
{
	register STATE_MACH_T   *stater;
	int iret, mret = 0;

	/* state machines per port */
    for (stater = this->primary->machines; stater; stater = stater->next) 
    {
        iret = (*iter_callb) (stater);
        if (exit_on_non_zero_ret && iret)
            return iret;
        else
            mret += iret;
    }

    for (stater = this->secondary->machines; stater; stater = stater->next) 
    {
        iret = (*iter_callb) (stater);
        if (exit_on_non_zero_ret && iret)
            return iret;
        else
            mret += iret;
    }
    
    /* state machines per ring */
	for (stater = this->machines; stater; stater = stater->next) {
		iret = (*iter_callb) (stater);
		if (exit_on_non_zero_ret && iret)
			return iret;
		else
			mret += iret;
	}

	return mret;
}

int RPP_ring_start (rppRing_t *this)
{
    RPP_port_start(this->primary);
    RPP_port_start(this->secondary);

	_rpp_ring_iterate_machines (this, _rpp_ring_init_machine, False);

	return 0;
}

int RPP_ring_stop (rppRing_t *this)
{
    this->status        = RPP_FAULT;
    this->node_role     = NODE_TYPE_NNKNOWN;
    this->switch_cnts   = 0;
    this->hello_syn     = 0;
    memset(&this->master_id,0,sizeof(NODE_ID_T));
    
    RPP_port_stop(this->primary);
    RPP_port_stop(this->secondary);

    return 0;
}

int RPP_ring_update (rppRing_t *this)
{
	register Bool need_state_change;
	register int  number_of_loops = 0;

	need_state_change = False; 

	for (;;) 
    {
		need_state_change = _rpp_ring_iterate_machines (this, RPP_check_condition, True);

		if (! need_state_change) 
			return number_of_loops;

		number_of_loops++;
		number_of_loops += _rpp_ring_iterate_machines (this, RPP_change_state, False);
	}

	return number_of_loops;
}


void RPP_ring_role_handler(rppRing_t *this,eNodeType role,NODE_ID_T *master_id)
{
    memcpy(&this->master_id,master_id,sizeof(NODE_ID_T));
    if(role == NODE_TYPE_MASTER)
    {
        this->switch_cnts   = 0;   
        this->hello_syn     = 0;
        this->node_role     = NODE_TYPE_MASTER;

        if(this->primary->neighber_vaild & this->secondary->neighber_vaild)
        {
            RPP_OUT_set_timer(&this->hello_timer,RPP_OUT_get_time(this,HELLO_TIME));
            RPP_OUT_set_timer(&this->fail_timer,RPP_OUT_get_time(this,FAIL_TIME));
        }
        else
        {
            if(this->primary->neighber_vaild)
                port_set_stp(this->primary,FORWARDING);
            if(this->secondary->neighber_vaild)
                port_set_stp(this->secondary,FORWARDING);
        }
    }
    else
    {
        this->node_role     =  NODE_TYPE_TRANSIT;
        RPP_OUT_set_timer(&this->fail_timer,RPP_OUT_get_time(this,FAIL_TIME));
    }
}

