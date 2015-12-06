#include "Kernel.h"

KAutoObject::KAutoObject() : m_refcount(0)
{

}
void KAutoObject::AcquireReference() {
    m_refcount++;
}

void KAutoObject::ReleaseReference() {
    m_refcount--;

    if(m_refcount == 0)
        Destroy();
}

void KAutoObject::Destroy() {
	delete this;
    // Empty. Overridden.
}

bool KAutoObject::IsInstanceOf(ClassName name) {
    return name == KAutoObject::name;
}
