#include "owner-cleanup.h"

#include "appstate.h"
#include "dataref.h"

void cleanupOwner(void *owner) {
    AppState::getInstance()->cancelTasksForOwner(owner);
    Dataref::getInstance()->unbindAll(owner);
}
