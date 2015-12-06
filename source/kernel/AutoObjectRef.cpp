#include "Kernel.h"

KAutoObjectRef::KAutoObjectRef(const KAutoObjectRef &obj) : m_object(obj.m_object)
{
	m_object->AcquireReference();
}

KAutoObjectRef::KAutoObjectRef(KAutoObject* object) {
    m_object = object;
    if (m_object)
        m_object->AcquireReference();
}

KAutoObjectRef::KAutoObjectRef() {
    m_object = NULL;
}

void KAutoObjectRef::SetObject(KAutoObject* object) {
    if(m_object != NULL) {
        m_object->ReleaseReference();
    }

    m_object = object;
    m_object->AcquireReference();
}

KAutoObjectRef::~KAutoObjectRef() {
    if(m_object != NULL)
        m_object->ReleaseReference();
}

KAutoObject* KAutoObjectRef::operator*() {
    return m_object;
}
