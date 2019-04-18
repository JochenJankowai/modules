#pragma once

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/tensorvisbase/ports/tensorfieldport.h>
#include <modules/tensorvisbase/datastructures/invariantspace.h>
#include <modules/tensorvisbase/tensorvisbasemoduledefine.h>

namespace inviwo {

class IVW_MODULE_TENSORVISBASE_API InvariantSpaceSelection : public Processor {
public:
    InvariantSpaceSelection();
    virtual ~InvariantSpaceSelection() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    TensorField3DInport tensorFieldInport_;

    InvariantSpaceOutport outport_;

    BoolProperty filterZeroes_;

    CompositeProperty invariantSpaceAxes_;

    BoolProperty sigma1_;
    BoolProperty sigma2_;
    BoolProperty sigma3_;

    BoolProperty i1_;
    BoolProperty i2_;
    BoolProperty i3_;

    BoolProperty j1_;
    BoolProperty j2_;
    BoolProperty j3_;

    BoolProperty lodeAngle_;

    BoolProperty anisotropy_;

    BoolProperty linearAnisotropy_;
    BoolProperty planarAnisotropy_;
    BoolProperty sphericalAnisotropy_;

    BoolProperty diffusivity_;
    BoolProperty shearStress_;
    BoolProperty pureShear_;
    BoolProperty shapeFactor_;
    BoolProperty isotropicScaling_;
    BoolProperty rotation_;
};

}  // namespace inviwo
