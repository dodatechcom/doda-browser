#ifndef DodaService_h_
#define DodaService_h_

#include "nsIDodaService.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "mozilla/Attributes.h"

namespace mozilla {
namespace sundaram {

class DodaService final : public nsIDodaService {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDODASERVICE

  DodaService();

 private:
  ~DodaService();
};

}  // namespace sundaram
}  // namespace mozilla

#endif  // DodaService_h_
