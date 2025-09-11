#pragma once
#include "mc/network/packet/PacketShapeData.h"
#include "mc/scripting/modules/minecraft/debugdrawer/ScriptDebugShapeType.h"

namespace debug_shape {

using ShapeID         = uint64_t;
using ShapeDataPacket = ScriptModuleDebugUtilities::PacketShapeData;
using DebugShapeType  = ScriptModuleDebugUtilities::ScriptDebugShapeType;

class IDrawerInterface {
public:
    virtual ~IDrawerInterface()                        = default; // vIndex: 0
    virtual void draw() const                          = 0;       // vIndex: 1
    virtual void draw(Player& player) const            = 0;       // vIndex: 2
    virtual void draw(DimensionType dimension) const   = 0;       // vIndex: 3
    virtual void remove() const                        = 0;       // vIndex: 4
    virtual void remove(Player& player) const          = 0;       // vIndex: 5
    virtual void remove(DimensionType dimension) const = 0;       // vIndex: 6
    virtual void update() const;                                  // vIndex: 7
    virtual void update(Player& player) const;                    // vIndex: 8
    virtual void update(DimensionType dimension) const;           // vIndex: 9
};

class IDebugShape : public IDrawerInterface {
protected:
    ShapeDataPacket mShapeData;

public:
    IDebugShape(const IDebugShape&)            = delete;
    IDebugShape& operator=(const IDebugShape&) = delete;

    // symbol: ??0IDebugShape@debug_shape@@QEAA@XZ
    IDebugShape();

    ~IDebugShape() override;                                                       // vIndex: 0
    virtual ShapeDataPacket const& serialize() const final;                        // vIndex: 1
    void                           draw() const override;                          // vIndex: 2
    void                           draw(Player& player) const override;            // vIndex: 3
    void                           draw(DimensionType dimension) const override;   // vIndex: 4
    void                           remove() const override;                        // vIndex: 5
    void                           remove(Player& player) const override;          // vIndex: 6
    void                           remove(DimensionType dimension) const override; // vIndex: 7
};

class DebugShape : public IDebugShape {
public:
    // symbol: ??0DebugShape@debug_shape@@QEAA@W4ScriptDebugShapeType@ScriptModuleDebugUtilities@@AEBVVec3@@@Z
    explicit DebugShape(DebugShapeType type, Vec3 const& loc);

    virtual ShapeID                   getId() const final;                      // vIndex: 0
    virtual DebugShapeType            getType() const final;                    // vIndex: 1
    virtual std::optional<Vec3>       getPosition() const;                      // vIndex: 2
    virtual void                      setPosition(Vec3 const& loc);             // vIndex: 3
    virtual std::optional<Vec3>       getRotation() const;                      // vIndex: 4
    virtual void                      setRotation(std::optional<Vec3> rot);     // vIndex: 5
    virtual std::optional<float>      getScale() const;                         // vIndex: 6
    virtual void                      setScale(std::optional<float> s);         // vIndex: 7
    virtual std::optional<mce::Color> getColor() const;                         // vIndex: 8
    virtual void                      setColor(std::optional<mce::Color> c);    // vIndex: 9
    virtual bool                      hasDuration() const;                      // vIndex: 10
    virtual std::optional<float>      getTotalTimeLeft() const;                 // vIndex: 11
    virtual void                      setTotalTimeLeft(std::optional<float> t); // vIndex: 12
};

class DebugLine : public DebugShape {
public:
    // symbol: ??0DebugLine@debug_shape@@QEAA@AEBVVec3@@0@Z
    explicit DebugLine(Vec3 const& start, Vec3 const& end);

    virtual std::optional<Vec3> getEndPosition() const;                  // vIndex: 0
    virtual void                setEndPosition(std::optional<Vec3> loc); // vIndex: 1
};

namespace extension {
class BoundsBox : public IDrawerInterface {
    std::array<std::unique_ptr<DebugLine>, 12> mLines{};

public:
    // symbol: ??1BoundsBox@extension@debug_shape@@UEAA@XZ
    explicit BoundsBox(AABB const& bounds, mce::Color const& color = mce::Color::WHITE());
    ~BoundsBox() override;                               // vIndex: 0
    void draw() const override;                          // vIndex: 1
    void draw(Player& player) const override;            // vIndex: 2
    void draw(DimensionType dimension) const override;   // vIndex: 3
    void remove() const override;                        // vIndex: 4
    void remove(Player& player) const override;          // vIndex: 5
    void remove(DimensionType dimension) const override; // vIndex: 6

    // void setBounds(AABB const& bounds);

    virtual std::optional<Vec3>       getRotation() const;                      // vIndex: 7
    virtual void                      setRotation(std::optional<Vec3> rot);     // vIndex: 8
    virtual std::optional<mce::Color> getColor() const;                         // vIndex: 9
    virtual void                      setColor(std::optional<mce::Color> c);    // vIndex: 10
    virtual bool                      hasDuration() const;                      // vIndex: 11
    virtual std::optional<float>      getTotalTimeLeft() const;                 // vIndex: 12
    virtual void                      setTotalTimeLeft(std::optional<float> t); // vIndex: 13
};
} // namespace extension

} // namespace debug_shape
