#include "source/common/upstream/default_local_address_selector_factory.h"

#include "source/common/upstream/default_local_address_selector.h"

namespace Envoy {
namespace Upstream {

namespace {
constexpr absl::string_view kDefaultLocalAddressSelectorName =
    "envoy.upstream.local_address_selector.default_local_address_selector";

void validate(const std::vector<::Envoy::Upstream::UpstreamLocalAddress>& upstream_local_addresses,
              absl::optional<std::string> cluster_name) {

  if (upstream_local_addresses.size() == 0) {
    throw EnvoyException(fmt::format("{}'s upstream binding config has no valid source address.",
                                     !(cluster_name.has_value())
                                         ? "Bootstrap"
                                         : fmt::format("Cluster {}", cluster_name.value())));
  }

  if (upstream_local_addresses.size() > 2) {
    throw EnvoyException(fmt::format(
        "{}'s upstream binding config has more than one extra/additional source addresses. Only "
        "one extra/additional source can be supported in BindConfig's "
        "extra_source_addresses/additional_source_addresses field",
        !(cluster_name.has_value()) ? "Bootstrap"
                                    : fmt::format("Cluster {}", cluster_name.value())));
  }
  // If we have exactly one upstream address, it needs to have a valid IP
  // version if non-null. This is enforced by the fact that BindConfig only
  // allows socket address.
  ASSERT(upstream_local_addresses.size() != 1 || upstream_local_addresses[0].address_ == nullptr ||
         upstream_local_addresses[0].address_->ip() != nullptr);

  // If we have more than one upstream address, they need to have different IP versions.
  if (upstream_local_addresses.size() == 2) {
    ASSERT(upstream_local_addresses[0].address_ != nullptr &&
           upstream_local_addresses[1].address_ != nullptr &&
           upstream_local_addresses[0].address_->ip() != nullptr &&
           upstream_local_addresses[1].address_->ip() != nullptr);

    if (upstream_local_addresses[0].address_->ip()->version() ==
        upstream_local_addresses[1].address_->ip()->version()) {
      throw EnvoyException(fmt::format(
          "{}'s upstream binding config has two same IP version source addresses. Only two "
          "different IP version source addresses can be supported in BindConfig's source_address "
          "and extra_source_addresses/additional_source_addresses fields",
          !(cluster_name.has_value()) ? "Bootstrap"
                                      : fmt::format("Cluster {}", cluster_name.value())));
    }
  }
}

} // namespace

std::string DefaultUpstreamLocalAddressSelectorFactory::name() const {
  return std::string(kDefaultLocalAddressSelectorName);
}

UpstreamLocalAddressSelectorConstSharedPtr
DefaultUpstreamLocalAddressSelectorFactory::createLocalAddressSelector(
    std::vector<::Envoy::Upstream::UpstreamLocalAddress> upstream_local_addresses,
    absl::optional<std::string> cluster_name) const {
  validate(upstream_local_addresses, cluster_name);
  return std::make_shared<DefaultUpstreamLocalAddressSelector>(std::move(upstream_local_addresses));
}

/**
 * Static registration for the default local address selector. @see RegisterFactory.
 */
REGISTER_FACTORY(DefaultUpstreamLocalAddressSelectorFactory, UpstreamLocalAddressSelectorFactory);

} // namespace Upstream
} // namespace Envoy
