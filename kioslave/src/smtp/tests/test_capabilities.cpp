#include <assert.h>
#include "capabilities.h"

using namespace KioSMTP;

int main() {
    Capabilities c;

    const QString size_cap = QObject::tr("SIZE 12");
    c.add(size_cap);
    // Capability was added
    assert(c.have("SIZE"));

    const QString expected_response = QObject::tr("SIZE=12");
    const QString actual_response = c.createSpecialResponse(false);
    // SIZE actually handled
    assert(actual_response == expected_response);

    const QString auth_cap = QObject::tr("AUTH GSSAPI");
    c.add(auth_cap);
    c.add(auth_cap);
    // Duplicate methods was removed
    assert(c.saslMethodsQSL().length() == 1);
}
