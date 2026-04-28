#include "TransformationState.h"
#include "Soldier.h"

TransformationState::TransformationState(int transformType)
    : type(transformType)
    , duration(10.0f)  // 10 seconds for both UNDEAD and MUMMY
{}

TransformationState::~TransformationState() {}

int TransformationState::getType() const {
    return this->type;
}

bool TransformationState::isExpired() const {
    return this->durationTimer.getElapsedTime().asSeconds() >= this->duration;
}
