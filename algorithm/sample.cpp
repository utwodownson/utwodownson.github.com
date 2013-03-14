for (;;) {
    ;
}

if () {
    ;
} else if () {
    ;
} else {
    ;
}


switch (reason) {

    case CLOCK_EVT_NOTIFY_ADD:
        return tick_check_new_device(dev);

    case CLOCK_EVT_NOTIFY_BROADCAST_ON:
    case CLOCK_EVT_NOTIFY_BROADCAST_OFF:
    case CLOCK_EVT_NOTIFY_BROADCAST_FORCE:
        tick_broadcast_on_off(reason, dev);
        break;

    case CLOCK_EVT_NOTIFY_BROADCAST_ENTER:
    case CLOCK_EVT_NOTIFY_BROADCAST_EXIT:
        tick_broadcast_oneshot_control(reason);
        break;

    case CLOCK_EVT_NOTIFY_CPU_DYING:
        tick_handover_do_timer(dev);
        break;

    case CLOCK_EVT_NOTIFY_RESUME:
        tick_resume();
        break;

    default:
        break;
}

int audit_tree_match(struct audit_chunk *chunk, struct audit_tree *tree)
{
    int n;
    for (n = 0; n < chunk->count; n++)
        if (chunk->owners[n].owner == tree)
            return 1;
    return 0;
}


struct region_devres match_data = { parent, start, n };
unsigned long owner, *p = (unsigned long *) &lock->owner;

