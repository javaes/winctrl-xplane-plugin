#ifndef OWNER_CLEANUP_H
#define OWNER_CLEANUP_H

// Safety net for the void* owner contract: anything that schedules tasks or
// monitors datarefs with `this` as owner must purge them on destruction, or
// the pending callbacks fire into a freed object. Profile base destructors
// call this so derived classes cannot forget. Both operations are idempotent,
// so explicit cleanup in derived destructors remains harmless.
//
// Note: relies on the base subobject sharing the derived object's address,
// which holds for single inheritance. Keep profile bases the first (only)
// base class.
void cleanupOwner(void *owner);

#endif
