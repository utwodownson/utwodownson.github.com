for (;;) 
{
    ;
}

if () 
{
    ;
} 
else if () 
{
    ;
} 
else 
{
    ;
}

// switch的标准格式
switch (reason) {

    case CLOCK_EVT_NOTIFY_ADD:
        return tick_check_new_device(dev);

    case CLOCK_EVT_NOTIFY_BROADCAST_ON:
    case CLOCK_EVT_NOTIFY_BROADCAST_OFF:
    case CLOCK_EVT_NOTIFY_BROADCAST_FORCE:
        tick_broadcast_on_off(reason, dev);
        break;

    case CLOCK_EVT_NOTIFY_CPU_DYING:
        tick_handover_do_timer(dev);
        break;

    default:
        break;
}

// for 循环的格式
int audit_tree_match(struct audit_chunk *chunk, struct audit_tree *tree)
{
    int n;
    for (n = 0; n < chunk->count; n++)
        if (chunk->owners[n].owner == tree)
            return 1;
    return 0;
}

// 结构体初始化需要在中括号间用空格
struct region_devres match_data = { parent, start, n };

//强制转换之间用空格
unsigned long owner, *p = (unsigned long *) &lock->owner; 

//这种替换写在一行
t = x[p]; x[p] = x[i]; x[i] = t; 

// 数组内的位移运算不空格
int  test(int i){ return a[i>>SHIFT] & (1<<(i & MASK)); }

