// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#pragma warning(disable : 4101)
#pragma warning(disable : 4065)
#endif

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wunreachable-code-break"
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

// includes
#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "Frame_m.h"

// register class
Register_Class(Frame)

    // ================================ FRAME ====================================
    // Frame class methods

    // calculate checksum
    void Frame::calculateCheckSum()
{
    byte checkSum(0);                         // Initialize checksum to 0
    std::string payloadStr = payload.c_str(); // Convert payload to a standard C++ string to make it easier to manipulate
    for (int i = 0; i < payloadStr.size(); i++)
    {
        byte character = byte(payloadStr[i]);                          // Convert char to binary
        unsigned int sum = checkSum.to_ulong() + character.to_ulong(); // Perform addition on integers

        if (sum > 255)
        {
            sum = (sum + 1) & 0xFF; // Wrap around and keep only the lower 8 bits
        }

        checkSum = byte(sum); // Convert back to byte
    }
    checkSum = ~checkSum; // 1's complement
    setTrailer(checkSum); // convert to bit stream and set trailer
}

// check checksum
bool Frame::checkCheckSum()
{
    Frame* frame = removeFraming(this);                    // Remove byte stuffing
    byte checkSum(frame->getTrailer());                   // Convert trailer to byte
    std::string payloadStr = std::string(frame->getPayload()).c_str();
    for (int i = 0; i < payloadStr.size(); i++)
    {
        byte character = byte(payloadStr[i]);                          // Convert char to binary
        unsigned int sum = checkSum.to_ulong() + character.to_ulong(); // Perform addition on integers

        if (sum > 255)
        {
            sum = (sum + 1) & 0xFF; // Wrap around and keep only the lower 8 bits
        }

        checkSum = byte(sum); // Convert back to byte
    }
    return ~byte(checkSum) == 0; // Check if all bits are 1
}

// applies byte stuffing to the payload and sets it in the payload
void Frame::applyFraming()
{
    std::string framedPayload;
    framedPayload += FLAG;                    // start flag
    std::string payloadStr = payload.c_str(); // Convert payload to a standard C++ string to make it easier to manipulate
    for (int i = 0; i < payloadStr.size(); i++)
    {
        char c = payloadStr[i];    // Get character
        if (c == FLAG || c == ESC) // If character is a flag or escape character, add escape character
        {
            framedPayload += ESC;
        }
        framedPayload += c; // Add character
    }
    framedPayload += FLAG;             // end flag
    setPayload(framedPayload.c_str()); // Set payload
}

// removes byte stuffing from the payload and sets it in the payload
Frame* Frame::removeFraming(Frame* frame)
{
    std::string unframedPayload;              // Initialize unframed payload
    std::string payloadStr = std::string(frame->getPayload()).c_str(); // Convert payload to a standard C++ string to make it easier to manipulate
    bool escape = false;                      // Initialize escape flag
    for (int i = 1; i < payloadStr.length() - 1; i++)
    {                           // Ignore first and last characters (FLAGS)
        char c = payloadStr[i]; // Get character
        if (escape)
        {
            unframedPayload += c; // Add character
            escape = false;
        }
        else if (c == ESC) // If character is escape character, set escape flag
        {
            escape = true;
        }
        else
        {
            unframedPayload += c; // Add character
        }
    }
    Frame* newFrame = frame->dup();
    newFrame->setPayload(unframedPayload.c_str()); // Set payload
    return newFrame;
}

void Frame::removeFraming()
{
    std::string unframedPayload;              // Initialize unframed payload
    std::string payloadStr = payload.c_str(); // Convert payload to a standard C++ string to make it easier to manipulate
    bool escape = false;                      // Initialize escape flag
    for (int i = 1; i < payloadStr.length() - 1; i++)
    {                           // Ignore first and last characters (FLAGS)
        char c = payloadStr[i]; // Get character
        if (escape)
        {
            unframedPayload += c; // Add character
            escape = false;
        }
        else if (c == ESC) // If character is escape character, set escape flag
        {
            escape = true;
        }
        else
        {
            unframedPayload += c; // Add character
        }
    }
    setPayload(unframedPayload.c_str()); // Set payload
}

// applies a random error to the frame
void Frame::applyRandomError(int modfiedBit)
{
    if (payload.empty()) // If payload is empty,
        return;
    // Convert payload to a standard C++ string to make it easier to manipulate
    std::string payloadStr = payload.c_str();
    // Generate a random index
    int index = modfiedBit / 8;
    // Generate a random bit position
    int bit = modfiedBit % 8;
    // Flip the bit at the generated position
    payloadStr[index] ^= (1 << bit);
    // Update the payload
    payload = payloadStr.c_str();
}

// sets the frame info
void Frame::setFrameInfo(int dataSeqNr, int type, int ackSeqNr, const char *payload, bool errored, int modfiedBit)
{
    this->dataSeqNr = dataSeqNr;      // Set frame info
    this->type = type;                // Set frame info
    this->ackSeqNr = ackSeqNr;        // Set frame info
    this->payload = payload;          // Set payload
    calculateCheckSum();              // Calculate checksum
    if (errored)                      // If errored,
        applyRandomError(modfiedBit); // Apply random error
    applyFraming();                   // Apply byte stuffing
}

// end frame class methods
// ============================================================================

namespace omnetpp
{

    // Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
    // They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

    // Packing/unpacking an std::vector
    template <typename T, typename A>
    void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T, A> &v)
    {
        int n = v.size();
        doParsimPacking(buffer, n);
        for (int i = 0; i < n; i++)
            doParsimPacking(buffer, v[i]);
    }

    template <typename T, typename A>
    void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T, A> &v)
    {
        int n;
        doParsimUnpacking(buffer, n);
        v.resize(n);
        for (int i = 0; i < n; i++)
            doParsimUnpacking(buffer, v[i]);
    }

    // Packing/unpacking an std::list
    template <typename T, typename A>
    void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T, A> &l)
    {
        doParsimPacking(buffer, (int)l.size());
        for (typename std::list<T, A>::const_iterator it = l.begin(); it != l.end(); ++it)
            doParsimPacking(buffer, (T &)*it);
    }

    template <typename T, typename A>
    void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T, A> &l)
    {
        int n;
        doParsimUnpacking(buffer, n);
        for (int i = 0; i < n; i++)
        {
            l.push_back(T());
            doParsimUnpacking(buffer, l.back());
        }
    }

    // Packing/unpacking an std::set
    template <typename T, typename Tr, typename A>
    void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T, Tr, A> &s)
    {
        doParsimPacking(buffer, (int)s.size());
        for (typename std::set<T, Tr, A>::const_iterator it = s.begin(); it != s.end(); ++it)
            doParsimPacking(buffer, *it);
    }

    template <typename T, typename Tr, typename A>
    void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T, Tr, A> &s)
    {
        int n;
        doParsimUnpacking(buffer, n);
        for (int i = 0; i < n; i++)
        {
            T x;
            doParsimUnpacking(buffer, x);
            s.insert(x);
        }
    }

    // Packing/unpacking an std::map
    template <typename K, typename V, typename Tr, typename A>
    void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K, V, Tr, A> &m)
    {
        doParsimPacking(buffer, (int)m.size());
        for (typename std::map<K, V, Tr, A>::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            doParsimPacking(buffer, it->first);
            doParsimPacking(buffer, it->second);
        }
    }

    template <typename K, typename V, typename Tr, typename A>
    void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K, V, Tr, A> &m)
    {
        int n;
        doParsimUnpacking(buffer, n);
        for (int i = 0; i < n; i++)
        {
            K k;
            V v;
            doParsimUnpacking(buffer, k);
            doParsimUnpacking(buffer, v);
            m[k] = v;
        }
    }

    // Default pack/unpack function for arrays
    template <typename T>
    void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
    {
        for (int i = 0; i < n; i++)
            doParsimPacking(b, t[i]);
    }

    template <typename T>
    void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
    {
        for (int i = 0; i < n; i++)
            doParsimUnpacking(b, t[i]);
    }

    // Default rule to prevent compiler from choosing base class' doParsimPacking() function
    template <typename T>
    void doParsimPacking(omnetpp::cCommBuffer *, const T &t)
    {
        throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
    }

    template <typename T>
    void doParsimUnpacking(omnetpp::cCommBuffer *, T &t)
    {
        throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
    }

} // namespace omnetpp

class byteDescriptor : public omnetpp::cClassDescriptor
{
private:
    mutable const char **propertyNames;
    enum FieldConstants
    {
    };

public:
    byteDescriptor();
    virtual ~byteDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue &value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(byteDescriptor)

    byteDescriptor::byteDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(byte)), "")
{
    propertyNames = nullptr;
}

byteDescriptor::~byteDescriptor()
{
    delete[] propertyNames;
}

bool byteDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<byte *>(obj) != nullptr;
}

const char **byteDescriptor::getPropertyNames() const
{
    if (!propertyNames)
    {
        static const char *names[] = {"existingClass", nullptr};
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *byteDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "existingClass"))
        return "";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int byteDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0 + base->getFieldCount() : 0;
}

unsigned int byteDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *byteDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int byteDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *byteDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **byteDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field)
    {
    default:
        return nullptr;
    }
}

const char *byteDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field)
    {
    default:
        return nullptr;
    }
}

int byteDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        return 0;
    }
}

void byteDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'byte'", field);
    }
}

const char *byteDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object, field, i);
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        return nullptr;
    }
}

std::string byteDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object, field, i);
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        return "";
    }
}

void byteDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        throw omnetpp::cRuntimeError("Cannot set field %d of class 'byte'", field);
    }
}

omnetpp::cValue byteDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldValue(object, field, i);
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        throw omnetpp::cRuntimeError("Cannot return field %d of class 'byte' as cValue -- field index out of range?", field);
    }
}

void byteDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue &value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        throw omnetpp::cRuntimeError("Cannot set field %d of class 'byte'", field);
    }
}

const char *byteDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr byteDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        return omnetpp::any_ptr(nullptr);
    }
}

void byteDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    byte *pp = omnetpp::fromAnyPtr<byte>(object);
    (void)pp;
    switch (field)
    {
    default:
        throw omnetpp::cRuntimeError("Cannot set field %d of class 'byte'", field);
    }
}

Frame_Base::Frame_Base(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

Frame_Base::Frame_Base(const Frame_Base &other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

Frame_Base::~Frame_Base()
{
}

Frame_Base &Frame_Base::operator=(const Frame_Base &other)
{
    if (this == &other)
        return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void Frame_Base::copy(const Frame_Base &other)
{
    this->dataSeqNr = other.dataSeqNr;
    this->type = other.type;
    this->ackSeqNr = other.ackSeqNr;
    this->payload = other.payload;
    this->trailer = other.trailer;
}

void Frame_Base::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b, this->dataSeqNr);
    doParsimPacking(b, this->type);
    doParsimPacking(b, this->ackSeqNr);
    doParsimPacking(b, this->payload);
    doParsimPacking(b, this->trailer);
}

void Frame_Base::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b, this->dataSeqNr);
    doParsimUnpacking(b, this->type);
    doParsimUnpacking(b, this->ackSeqNr);
    doParsimUnpacking(b, this->payload);
    doParsimUnpacking(b, this->trailer);
}

int Frame_Base::getDataSeqNr() const
{
    return this->dataSeqNr;
}

void Frame_Base::setDataSeqNr(int dataSeqNr)
{
    this->dataSeqNr = dataSeqNr;
}

int Frame_Base::getType() const
{
    return this->type;
}

void Frame_Base::setType(int type)
{
    this->type = type;
}

int Frame_Base::getAckSeqNr() const
{
    return this->ackSeqNr;
}

void Frame_Base::setAckSeqNr(int ackSeqNr)
{
    this->ackSeqNr = ackSeqNr;
}

const char *Frame_Base::getPayload() const
{
    return this->payload.c_str();
}

void Frame_Base::setPayload(const char *payload)
{
    this->payload = payload;
}

const byte &Frame_Base::getTrailer() const
{
    return this->trailer;
}

void Frame_Base::setTrailer(const byte &trailer)
{
    this->trailer = trailer;
}

class FrameDescriptor : public omnetpp::cClassDescriptor
{
private:
    mutable const char **propertyNames;
    enum FieldConstants
    {
        FIELD_dataSeqNr,
        FIELD_type,
        FIELD_ackSeqNr,
        FIELD_payload,
        FIELD_trailer,
    };

public:
    FrameDescriptor();
    virtual ~FrameDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue &value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(FrameDescriptor)

    FrameDescriptor::FrameDescriptor() : omnetpp::cClassDescriptor("Frame", "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

FrameDescriptor::~FrameDescriptor()
{
    delete[] propertyNames;
}

bool FrameDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Frame_Base *>(obj) != nullptr;
}

const char **FrameDescriptor::getPropertyNames() const
{
    if (!propertyNames)
    {
        static const char *names[] = {"customize", nullptr};
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *FrameDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "customize"))
        return "true";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int FrameDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5 + base->getFieldCount() : 5;
}

unsigned int FrameDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE, // FIELD_dataSeqNr
        FD_ISEDITABLE, // FIELD_type
        FD_ISEDITABLE, // FIELD_ackSeqNr
        FD_ISEDITABLE, // FIELD_payload
        FD_ISCOMPOUND, // FIELD_trailer
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *FrameDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dataSeqNr",
        "type",
        "ackSeqNr",
        "payload",
        "trailer",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int FrameDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dataSeqNr") == 0)
        return baseIndex + 0;
    if (strcmp(fieldName, "type") == 0)
        return baseIndex + 1;
    if (strcmp(fieldName, "ackSeqNr") == 0)
        return baseIndex + 2;
    if (strcmp(fieldName, "payload") == 0)
        return baseIndex + 3;
    if (strcmp(fieldName, "trailer") == 0)
        return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *FrameDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_dataSeqNr
        "int",    // FIELD_type
        "int",    // FIELD_ackSeqNr
        "string", // FIELD_payload
        "byte",   // FIELD_trailer
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **FrameDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field)
    {
    default:
        return nullptr;
    }
}

const char *FrameDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field)
    {
    default:
        return nullptr;
    }
}

int FrameDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    default:
        return 0;
    }
}

void FrameDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    default:
        throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'Frame_Base'", field);
    }
}

const char *FrameDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object, field, i);
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    default:
        return nullptr;
    }
}

std::string FrameDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object, field, i);
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    case FIELD_dataSeqNr:
        return long2string(pp->getDataSeqNr());
    case FIELD_type:
        return long2string(pp->getType());
    case FIELD_ackSeqNr:
        return long2string(pp->getAckSeqNr());
    case FIELD_payload:
        return oppstring2string(pp->getPayload());
    case FIELD_trailer:
        return "";
    default:
        return "";
    }
}

void FrameDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    case FIELD_dataSeqNr:
        pp->setDataSeqNr(string2long(value));
        break;
    case FIELD_type:
        pp->setType(string2long(value));
        break;
    case FIELD_ackSeqNr:
        pp->setAckSeqNr(string2long(value));
        break;
    case FIELD_payload:
        pp->setPayload((value));
        break;
    default:
        throw omnetpp::cRuntimeError("Cannot set field %d of class 'Frame_Base'", field);
    }
}

omnetpp::cValue FrameDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldValue(object, field, i);
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    case FIELD_dataSeqNr:
        return pp->getDataSeqNr();
    case FIELD_type:
        return pp->getType();
    case FIELD_ackSeqNr:
        return pp->getAckSeqNr();
    case FIELD_payload:
        return pp->getPayload();
    case FIELD_trailer:
        return omnetpp::toAnyPtr(&pp->getTrailer());
        break;
    default:
        throw omnetpp::cRuntimeError("Cannot return field %d of class 'Frame_Base' as cValue -- field index out of range?", field);
    }
}

void FrameDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue &value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    case FIELD_dataSeqNr:
        pp->setDataSeqNr(omnetpp::checked_int_cast<int>(value.intValue()));
        break;
    case FIELD_type:
        pp->setType(omnetpp::checked_int_cast<int>(value.intValue()));
        break;
    case FIELD_ackSeqNr:
        pp->setAckSeqNr(omnetpp::checked_int_cast<int>(value.intValue()));
        break;
    case FIELD_payload:
        pp->setPayload(value.stringValue());
        break;
    default:
        throw omnetpp::cRuntimeError("Cannot set field %d of class 'Frame_Base'", field);
    }
}

const char *FrameDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field)
    {
    case FIELD_trailer:
        return omnetpp::opp_typename(typeid(byte));
    default:
        return nullptr;
    };
}

omnetpp::any_ptr FrameDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    case FIELD_trailer:
        return omnetpp::toAnyPtr(&pp->getTrailer());
        break;
    default:
        return omnetpp::any_ptr(nullptr);
    }
}

void FrameDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base)
    {
        if (field < base->getFieldCount())
        {
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    Frame_Base *pp = omnetpp::fromAnyPtr<Frame_Base>(object);
    (void)pp;
    switch (field)
    {
    default:
        throw omnetpp::cRuntimeError("Cannot set field %d of class 'Frame_Base'", field);
    }
}

namespace omnetpp
{

} // namespace omnetpp
