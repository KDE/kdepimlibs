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
}
