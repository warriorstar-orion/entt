#include <algorithm>
#include <unordered_map>
#include <vector>
#include <gtest/gtest.h>
#include <entt/core/any.hpp>

struct fat {
    double value[4];
    inline static int counter = 0;

    ~fat() { ++counter; }

    bool operator==(const fat &other) const {
        return std::equal(std::begin(value), std::end(value), std::begin(other.value), std::end(other.value));
    }
};

struct empty {
    inline static int counter = 0;
    ~empty() { ++counter; }
};

struct not_comparable {
    bool operator==(const not_comparable &) const = delete;
};

template<auto Sz>
struct not_copyable {
    not_copyable(): payload{} {}
    not_copyable(const not_copyable &) = delete;
    not_copyable(not_copyable &&) = default;
    not_copyable & operator=(const not_copyable &) = delete;
    not_copyable & operator=(not_copyable &&) = default;
    double payload[Sz];
};

struct Any: ::testing::Test {
    void SetUp() override {
        fat::counter = 0;
        empty::counter = 0;
    }
};

TEST_F(Any, SBO) {
    entt::any any{'c'};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<char>());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<char>(any), 'c');
}

TEST_F(Any, NoSBO) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{instance};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(any), instance);
}

TEST_F(Any, Empty) {
    entt::any any{};

    ASSERT_FALSE(any);
    ASSERT_FALSE(any.type());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(any.data(), nullptr);
}

TEST_F(Any, SBOInPlaceTypeConstruction) {
    entt::any any{entt::in_place_type<int>, 42};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<int>(any), 42);

    auto other = as_ref(any);

    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(other), 42);
    ASSERT_EQ(other.data(), any.data());
}

TEST_F(Any, SBOAsRefConstruction) {
    int value = 42;
    entt::any any{std::ref(value)};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<int>());

    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<const int>(&any), &value);
    ASSERT_EQ(entt::any_cast<int>(&any), &value);
    ASSERT_EQ(entt::any_cast<const int>(&std::as_const(any)), &value);
    ASSERT_EQ(entt::any_cast<int>(&std::as_const(any)), &value);

    ASSERT_EQ(entt::any_cast<const int &>(any), 42);
    ASSERT_EQ(entt::any_cast<int>(any), 42);

    ASSERT_EQ(any.data(), &value);
    ASSERT_EQ(std::as_const(any).data(), &value);

    auto other = as_ref(any);

    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(other), 42);
    ASSERT_EQ(other.data(), any.data());
}

TEST_F(Any, SBOAsConstRefConstruction) {
    int value = 42;
    entt::any any{std::cref(value)};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<int>());

    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<const int>(&any), &value);
    ASSERT_EQ(entt::any_cast<int>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<const int>(&std::as_const(any)), &value);
    ASSERT_EQ(entt::any_cast<int>(&std::as_const(any)), &value);

    ASSERT_EQ(entt::any_cast<const int &>(any), 42);
    ASSERT_EQ(entt::any_cast<int>(any), 42);

    ASSERT_EQ(any.data(), nullptr);
    ASSERT_EQ(std::as_const(any).data(), &value);

    auto other = as_ref(any);

    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(other), 42);
    ASSERT_EQ(other.data(), any.data());
}

TEST_F(Any, SBOCopyConstruction) {
    entt::any any{42};
    entt::any other{any};

    ASSERT_TRUE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(any.type(), entt::type_id<int>());
    ASSERT_EQ(other.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<int>(other), 42);
}

TEST_F(Any, SBOCopyAssignment) {
    entt::any any{42};
    entt::any other{3};

    other = any;

    ASSERT_TRUE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(any.type(), entt::type_id<int>());
    ASSERT_EQ(other.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<int>(other), 42);
}

TEST_F(Any, SBOMoveConstruction) {
    entt::any any{42};
    entt::any other{std::move(any)};

    ASSERT_FALSE(any);
    ASSERT_TRUE(other);
    ASSERT_FALSE(any.type());
    ASSERT_EQ(other.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<int>(other), 42);
}

TEST_F(Any, SBOMoveAssignment) {
    entt::any any{42};
    entt::any other{3};

    other = std::move(any);

    ASSERT_FALSE(any);
    ASSERT_TRUE(other);
    ASSERT_FALSE(any.type());
    ASSERT_EQ(other.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<int>(other), 42);
}

TEST_F(Any, SBODirectAssignment) {
    entt::any any{};
    any = 42;

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<int>(any), 42);
}

TEST_F(Any, NoSBOInPlaceTypeConstruction) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{entt::in_place_type<fat>, instance};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(any), instance);

    auto other = as_ref(any);

    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<fat>(other), (fat{{.1, .2, .3, .4}}));
    ASSERT_EQ(other.data(), any.data());
}

TEST_F(Any, NoSBOAsRefConstruction) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{std::ref(instance)};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<fat>());

    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<const fat>(&any), &instance);
    ASSERT_EQ(entt::any_cast<fat>(&any), &instance);
    ASSERT_EQ(entt::any_cast<const fat>(&std::as_const(any)), &instance);
    ASSERT_EQ(entt::any_cast<fat>(&std::as_const(any)), &instance);

    ASSERT_EQ(entt::any_cast<const fat &>(any), instance);
    ASSERT_EQ(entt::any_cast<fat>(any), instance);

    ASSERT_EQ(any.data(), &instance);
    ASSERT_EQ(std::as_const(any).data(), &instance);

    auto other = as_ref(any);

    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<fat>(other), (fat{{.1, .2, .3, .4}}));
    ASSERT_EQ(other.data(), any.data());
}

TEST_F(Any, NoSBOAsConstRefConstruction) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{std::cref(instance)};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<fat>());

    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<const fat>(&any), &instance);
    ASSERT_EQ(entt::any_cast<fat>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<const fat>(&std::as_const(any)), &instance);
    ASSERT_EQ(entt::any_cast<fat>(&std::as_const(any)), &instance);

    ASSERT_EQ(entt::any_cast<const fat &>(any), instance);
    ASSERT_EQ(entt::any_cast<fat>(any), instance);

    ASSERT_EQ(any.data(), nullptr);
    ASSERT_EQ(std::as_const(any).data(), &instance);

    auto other = as_ref(any);

    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<fat>(other), (fat{{.1, .2, .3, .4}}));
    ASSERT_EQ(other.data(), any.data());
}

TEST_F(Any, NoSBOCopyConstruction) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{instance};
    entt::any other{any};

    ASSERT_TRUE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(any.type(), entt::type_id<fat>());
    ASSERT_EQ(other.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(other), instance);
}

TEST_F(Any, NoSBOCopyAssignment) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{instance};
    entt::any other{3};

    other = any;

    ASSERT_TRUE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(any.type(), entt::type_id<fat>());
    ASSERT_EQ(other.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(other), instance);
}

TEST_F(Any, NoSBOMoveConstruction) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{instance};
    entt::any other{std::move(any)};

    ASSERT_FALSE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(other), instance);
}

TEST_F(Any, NoSBOMoveAssignment) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{instance};
    entt::any other{3};

    other = std::move(any);

    ASSERT_FALSE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(other.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(other), instance);
}

TEST_F(Any, NoSBODirectAssignment) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{};
    any = instance;

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(any), instance);
}

TEST_F(Any, VoidInPlaceTypeConstruction) {
    entt::any any{entt::in_place_type<void>};

    ASSERT_FALSE(any);
    ASSERT_FALSE(any.type());
    ASSERT_EQ(entt::any_cast<int>(&any), nullptr);
}

TEST_F(Any, VoidCopyConstruction) {
    entt::any any{entt::in_place_type<void>};
    entt::any other{any};

    ASSERT_FALSE(any);
    ASSERT_FALSE(other);
    ASSERT_FALSE(any.type());
    ASSERT_FALSE(other.type());
    ASSERT_EQ(entt::any_cast<int>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
}

TEST_F(Any, VoidCopyAssignment) {
    entt::any any{entt::in_place_type<void>};
    entt::any other{entt::in_place_type<void>};

    other = any;

    ASSERT_FALSE(any);
    ASSERT_FALSE(other);
    ASSERT_FALSE(any.type());
    ASSERT_FALSE(other.type());
    ASSERT_EQ(entt::any_cast<int>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
}

TEST_F(Any, VoidMoveConstruction) {
    entt::any any{entt::in_place_type<void>};
    entt::any other{std::move(any)};

    ASSERT_FALSE(any);
    ASSERT_FALSE(other);
    ASSERT_FALSE(any.type());
    ASSERT_FALSE(other.type());
    ASSERT_EQ(entt::any_cast<int>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
}

TEST_F(Any, VoidMoveAssignment) {
    entt::any any{entt::in_place_type<void>};
    entt::any other{entt::in_place_type<void>};

    other = std::move(any);

    ASSERT_FALSE(any);
    ASSERT_FALSE(other);
    ASSERT_FALSE(any.type());
    ASSERT_FALSE(other.type());
    ASSERT_EQ(entt::any_cast<int>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&other), nullptr);
}

TEST_F(Any, SBOMoveInvalidate) {
    entt::any any{42};
    entt::any other{std::move(any)};
    entt::any valid = std::move(other);

    ASSERT_FALSE(any);
    ASSERT_FALSE(other);
    ASSERT_TRUE(valid);
}

TEST_F(Any, NoSBOMoveInvalidate) {
    fat instance{{.1, .2, .3, .4}};
    entt::any any{instance};
    entt::any other{std::move(any)};
    entt::any valid = std::move(other);

    ASSERT_FALSE(any);
    ASSERT_FALSE(other);
    ASSERT_TRUE(valid);
}

TEST_F(Any, VoidMoveInvalidate) {
    entt::any any{entt::in_place_type<void>};
    entt::any other{std::move(any)};
    entt::any valid = std::move(other);

    ASSERT_FALSE(any);
    ASSERT_FALSE(other);
    ASSERT_FALSE(valid);
}

TEST_F(Any, SBODestruction) {
    {
        entt::any any{empty{}};
        empty::counter = 0;
    }

    ASSERT_EQ(empty::counter, 1);
}

TEST_F(Any, NoSBODestruction) {
    {
        entt::any any{fat{}};
        fat::counter = 0;
    }

    ASSERT_EQ(fat::counter, 1);
}

TEST_F(Any, VoidDestruction) {
    // just let asan tell us if everything is ok here
    [[maybe_unused]] entt::any any{entt::in_place_type<void>};
}

TEST_F(Any, Emplace) {
    entt::any any{};
    any.emplace<int>(42);

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<double>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<int>(any), 42);
}

TEST_F(Any, EmplaceVoid) {
    entt::any any{};
    any.emplace<void>();

    ASSERT_FALSE(any);
    ASSERT_FALSE(any.type());
 }

TEST_F(Any, Reset) {
    entt::any any{42};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<int>());

    any.reset();

    ASSERT_FALSE(any);
    ASSERT_EQ(any.type(), entt::type_info{});
}

TEST_F(Any, SBOSwap) {
    entt::any lhs{'c'};
    entt::any rhs{42};

    std::swap(lhs, rhs);

    ASSERT_EQ(lhs.type(), entt::type_id<int>());
    ASSERT_EQ(rhs.type(), entt::type_id<char>());
    ASSERT_EQ(entt::any_cast<char>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<int>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<int>(lhs), 42);
    ASSERT_EQ(entt::any_cast<char>(rhs), 'c');
}

TEST_F(Any, NoSBOSwap) {
    entt::any lhs{fat{{.1, .2, .3, .4}}};
    entt::any rhs{fat{{.4, .3, .2, .1}}};

    std::swap(lhs, rhs);

    ASSERT_EQ(entt::any_cast<fat>(lhs), (fat{{.4, .3, .2, .1}}));
    ASSERT_EQ(entt::any_cast<fat>(rhs), (fat{{.1, .2, .3, .4}}));
}

TEST_F(Any, VoidSwap) {
    entt::any lhs{entt::in_place_type<void>};
    entt::any rhs{entt::in_place_type<void>};
    const auto *pre = lhs.data();

    std::swap(lhs, rhs);

    ASSERT_EQ(pre, lhs.data());
}

TEST_F(Any, SBOWithNoSBOSwap) {
    entt::any lhs{fat{{.1, .2, .3, .4}}};
    entt::any rhs{'c'};

    std::swap(lhs, rhs);

    ASSERT_EQ(lhs.type(), entt::type_id<char>());
    ASSERT_EQ(rhs.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<fat>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(lhs), 'c');
    ASSERT_EQ(entt::any_cast<fat>(rhs), (fat{{.1, .2, .3, .4}}));
}

TEST_F(Any, SBOWithRefSwap) {
    int value = 3;
    entt::any lhs{std::ref(value)};
    entt::any rhs{'c'};

    std::swap(lhs, rhs);

    ASSERT_EQ(lhs.type(), entt::type_id<char>());
    ASSERT_EQ(rhs.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(lhs), 'c');
    ASSERT_EQ(entt::any_cast<int>(rhs), 3);
    ASSERT_EQ(rhs.data(), &value);
}

TEST_F(Any, SBOWithConstRefSwap) {
    int value = 3;
    entt::any lhs{std::cref(value)};
    entt::any rhs{'c'};

    std::swap(lhs, rhs);

    ASSERT_EQ(lhs.type(), entt::type_id<char>());
    ASSERT_EQ(rhs.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(lhs), 'c');
    ASSERT_EQ(entt::any_cast<int>(rhs), 3);
    ASSERT_EQ(rhs.data(), nullptr);
    ASSERT_EQ(std::as_const(rhs).data(), &value);
}

TEST_F(Any, SBOWithEmptySwap) {
    entt::any lhs{'c'};
    entt::any rhs{};

    std::swap(lhs, rhs);

    ASSERT_FALSE(lhs);
    ASSERT_EQ(rhs.type(), entt::type_id<char>());
    ASSERT_EQ(entt::any_cast<char>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(rhs), 'c');

    std::swap(lhs, rhs);

    ASSERT_FALSE(rhs);
    ASSERT_EQ(lhs.type(), entt::type_id<char>());
    ASSERT_EQ(entt::any_cast<double>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(lhs), 'c');
}

TEST_F(Any, SBOWithVoidSwap) {
    entt::any lhs{'c'};
    entt::any rhs{entt::in_place_type<void>};

    std::swap(lhs, rhs);

    ASSERT_FALSE(lhs);
    ASSERT_EQ(rhs.type(), entt::type_id<char>());
    ASSERT_EQ(entt::any_cast<char>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(rhs), 'c');

    std::swap(lhs, rhs);

    ASSERT_FALSE(rhs);
    ASSERT_EQ(lhs.type(), entt::type_id<char>());
    ASSERT_EQ(entt::any_cast<double>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<char>(lhs), 'c');
}

TEST_F(Any, NoSBOWithRefSwap) {
    int value = 3;
    entt::any lhs{std::ref(value)};
    entt::any rhs{fat{{.1, .2, .3, .4}}};

    std::swap(lhs, rhs);

    ASSERT_EQ(lhs.type(), entt::type_id<fat>());
    ASSERT_EQ(rhs.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(lhs), (fat{{.1, .2, .3, .4}}));
    ASSERT_EQ(entt::any_cast<int>(rhs), 3);
    ASSERT_EQ(rhs.data(), &value);
}

TEST_F(Any, NoSBOWithConstRefSwap) {
    int value = 3;
    entt::any lhs{std::cref(value)};
    entt::any rhs{fat{{.1, .2, .3, .4}}};

    std::swap(lhs, rhs);

    ASSERT_EQ(lhs.type(), entt::type_id<fat>());
    ASSERT_EQ(rhs.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(lhs), (fat{{.1, .2, .3, .4}}));
    ASSERT_EQ(entt::any_cast<int>(rhs), 3);
    ASSERT_EQ(rhs.data(), nullptr);
    ASSERT_EQ(std::as_const(rhs).data(), &value);
}

TEST_F(Any, NoSBOWithEmptySwap) {
    entt::any lhs{fat{{.1, .2, .3, .4}}};
    entt::any rhs{};

    std::swap(lhs, rhs);

    ASSERT_FALSE(lhs);
    ASSERT_EQ(rhs.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<fat>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(rhs), (fat{{.1, .2, .3, .4}}));

    std::swap(lhs, rhs);

    ASSERT_FALSE(rhs);
    ASSERT_EQ(lhs.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(lhs), (fat{{.1, .2, .3, .4}}));
}

TEST_F(Any, NoSBOWithVoidSwap) {
    entt::any lhs{fat{{.1, .2, .3, .4}}};
    entt::any rhs{entt::in_place_type<void>};

    std::swap(lhs, rhs);

    ASSERT_FALSE(lhs);
    ASSERT_EQ(rhs.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<fat>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<double>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(rhs), (fat{{.1, .2, .3, .4}}));

    std::swap(lhs, rhs);

    ASSERT_FALSE(rhs);
    ASSERT_EQ(lhs.type(), entt::type_id<fat>());
    ASSERT_EQ(entt::any_cast<double>(&lhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(&rhs), nullptr);
    ASSERT_EQ(entt::any_cast<fat>(lhs), (fat{{.1, .2, .3, .4}}));
}

TEST_F(Any, AsRef) {
    entt::any any{42};
    auto ref = as_ref(any);
    auto cref = as_ref(std::as_const(any));

    ASSERT_EQ(entt::any_cast<int>(&any), any.data());
    ASSERT_EQ(entt::any_cast<int>(&ref), any.data());
    ASSERT_EQ(entt::any_cast<int>(&cref), nullptr);

    ASSERT_EQ(entt::any_cast<const int>(&any), any.data());
    ASSERT_EQ(entt::any_cast<const int>(&ref), any.data());
    ASSERT_EQ(entt::any_cast<const int>(&cref), any.data());

    ASSERT_EQ(entt::any_cast<int>(any), 42);
    ASSERT_EQ(entt::any_cast<int>(ref), 42);
    ASSERT_EQ(entt::any_cast<int>(cref), 42);

    ASSERT_EQ(entt::any_cast<const int>(any), 42);
    ASSERT_EQ(entt::any_cast<const int>(ref), 42);
    ASSERT_EQ(entt::any_cast<const int>(cref), 42);

    ASSERT_EQ(entt::any_cast<int &>(any), 42);
    ASSERT_EQ(entt::any_cast<const int &>(any), 42);
    ASSERT_EQ(entt::any_cast<int &>(ref), 42);
    ASSERT_EQ(entt::any_cast<const int &>(ref), 42);
    ASSERT_DEATH(entt::any_cast<int &>(cref), ".*");
    ASSERT_EQ(entt::any_cast<const int &>(cref), 42);

    entt::any_cast<int &>(any) = 3;

    ASSERT_EQ(entt::any_cast<int>(any), 3);
    ASSERT_EQ(entt::any_cast<int>(ref), 3);
    ASSERT_EQ(entt::any_cast<int>(cref), 3);

    std::swap(ref, cref);

    ASSERT_EQ(entt::any_cast<int>(&ref), nullptr);
    ASSERT_EQ(entt::any_cast<int>(&cref), any.data());

    ref = as_ref(ref);
    cref = as_ref(std::as_const(cref));

    ASSERT_EQ(entt::any_cast<int>(&ref), nullptr);
    ASSERT_EQ(entt::any_cast<int>(&cref), nullptr);
    ASSERT_EQ(entt::any_cast<const int>(&ref), any.data());
    ASSERT_EQ(entt::any_cast<const int>(&cref), any.data());

    ASSERT_DEATH(entt::any_cast<int &>(ref), ".*");
    ASSERT_DEATH(entt::any_cast<int &>(cref), ".*");

    ASSERT_EQ(entt::any_cast<const int &>(ref), 3);
    ASSERT_EQ(entt::any_cast<const int &>(cref), 3);

    ref = 42;
    cref = 42;

    ASSERT_NE(entt::any_cast<int>(&ref), nullptr);
    ASSERT_NE(entt::any_cast<int>(&cref), nullptr);
    ASSERT_EQ(entt::any_cast<int &>(ref), 42);
    ASSERT_EQ(entt::any_cast<int &>(cref), 42);
    ASSERT_EQ(entt::any_cast<const int &>(ref), 42);
    ASSERT_EQ(entt::any_cast<const int &>(cref), 42);
    ASSERT_NE(entt::any_cast<int>(&ref), any.data());
    ASSERT_NE(entt::any_cast<int>(&cref), any.data());
}

TEST_F(Any, Comparable) {
    auto test = [](entt::any any, entt::any other) {
        ASSERT_EQ(any, any);
        ASSERT_NE(other, any);
        ASSERT_NE(any, entt::any{});

        ASSERT_TRUE(any == any);
        ASSERT_FALSE(other == any);
        ASSERT_TRUE(any != other);
        ASSERT_TRUE(entt::any{} != any);
    };

    int value = 42;

    test('c', 'a');
    test(fat{{.1, .2, .3, .4}}, fat{{.0, .1, .2, .3}});
    test(std::ref(value), 3);
    test(3, std::cref(value));
}

TEST_F(Any, NotComparable) {
    auto test = [](const auto &instance) {
        entt::any any{std::cref(instance)};

        ASSERT_EQ(any, any);
        ASSERT_NE(any, entt::any{instance});
        ASSERT_NE(entt::any{}, any);

        ASSERT_TRUE(any == any);
        ASSERT_FALSE(any == entt::any{instance});
        ASSERT_TRUE(entt::any{} != any);
    };

    test(not_comparable{});
    test(std::unordered_map<int, not_comparable>{});
    test(std::vector<not_comparable>{});
}

TEST_F(Any, CompareVoid) {
    entt::any any{entt::in_place_type<void>};

    ASSERT_EQ(any, any);
    ASSERT_EQ(any, entt::any{entt::in_place_type<void>});
    ASSERT_NE(entt::any{'a'}, any);
    ASSERT_EQ(any, entt::any{});

    ASSERT_TRUE(any == any);
    ASSERT_TRUE(any == entt::any{entt::in_place_type<void>});
    ASSERT_FALSE(entt::any{'a'} == any);
    ASSERT_TRUE(any != entt::any{'a'});
    ASSERT_FALSE(entt::any{} != any);
}

TEST_F(Any, AnyCast) {
    entt::any any{42};
    const auto &cany = any;

    ASSERT_EQ(entt::any_cast<char>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<char>(&cany), nullptr);
    ASSERT_EQ(*entt::any_cast<int>(&any), 42);
    ASSERT_EQ(*entt::any_cast<int>(&cany), 42);
    ASSERT_EQ(entt::any_cast<int &>(any), 42);
    ASSERT_EQ(entt::any_cast<const int &>(cany), 42);
    ASSERT_EQ(entt::any_cast<int>(42), 42);
}

TEST_F(Any, NotCopyableType) {
    auto test = [](entt::any any) {
        entt::any copy{any};

        ASSERT_TRUE(any);
        ASSERT_FALSE(copy);

        copy = any;

        ASSERT_TRUE(any);
        ASSERT_FALSE(copy);
    };

    test(entt::any{entt::in_place_type<not_copyable<1>>});
    test(entt::any{entt::in_place_type<not_copyable<4>>});
}

TEST_F(Any, Array) {
    entt::any any{entt::in_place_type<int[1]>};
    entt::any copy{any};

    ASSERT_TRUE(any);
    ASSERT_FALSE(copy);

    ASSERT_EQ(any.type(), entt::type_id<int[1]>());
    ASSERT_NE(entt::any_cast<int[1]>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<int[2]>(&any), nullptr);
    ASSERT_EQ(entt::any_cast<int *>(&any), nullptr);

    entt::any_cast<int(&)[1]>(any)[0] = 42;

    ASSERT_EQ(entt::any_cast<const int(&)[1]>(std::as_const(any))[0], 42);
}

TEST_F(Any, SBOAsDyn) {
    entt::any any{entt::in_place_type<int>, 42};
    entt::any other{entt::in_place_type_dyn<int>, 3};

    ASSERT_TRUE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(any.type(), entt::type_id<int>());
    ASSERT_EQ(any.type(), other.type());
    ASSERT_EQ(entt::any_cast<int>(any), 42);
    ASSERT_EQ(entt::any_cast<int>(other), 3);

    std::swap(any, other);

    ASSERT_EQ(entt::any_cast<int &>(any), 3);
    ASSERT_EQ(entt::any_cast<int &>(other), 42);
    ASSERT_FALSE(any == other);
    ASSERT_TRUE(any != other);

    auto alias = as_ref(any);
    entt::any_cast<int &>(alias) = 0;

    ASSERT_EQ(entt::any_cast<int>(alias), 0);
    ASSERT_EQ(entt::any_cast<int>(alias), entt::any_cast<int>(any));

    any.reset();

    ASSERT_FALSE(any);
}

TEST_F(Any, NoSBOAsDyn) {
    entt::any any{entt::in_place_type<fat>, fat{{.1, .2, .3, .4}}};
    entt::any other{entt::in_place_type_dyn<fat>, fat{{.0, .0, .0, .0}}};

    ASSERT_TRUE(any);
    ASSERT_TRUE(other);
    ASSERT_EQ(fat::counter, 2);
    ASSERT_EQ(any.type(), entt::type_id<fat>());
    ASSERT_EQ(any.type(), other.type());
    ASSERT_EQ(entt::any_cast<fat>(any).value[0], .1);
    ASSERT_EQ(entt::any_cast<fat>(other).value[0], .0);

    std::swap(any, other);

    ASSERT_EQ(fat::counter, 4);
    ASSERT_EQ(entt::any_cast<fat &>(any).value[1], .0);
    ASSERT_EQ(entt::any_cast<fat &>(other).value[1], .2);
    ASSERT_FALSE(any == other);
    ASSERT_TRUE(any != other);

    auto alias = as_ref(any);
    entt::any_cast<fat &>(alias).value[3] = .5;

    ASSERT_EQ(entt::any_cast<fat>(alias).value[3], .5);
    ASSERT_EQ(entt::any_cast<fat>(alias), entt::any_cast<fat>(any));

    any.reset();

    ASSERT_FALSE(any);
}

TEST_F(Any, RefAsDyn) {
    int value = 42;
    entt::any any{entt::in_place_type_dyn<int &>, value};

    ASSERT_TRUE(any);
    ASSERT_EQ(any.type(), entt::type_id<int>());
    ASSERT_EQ(entt::any_cast<int>(any), 42);
    ASSERT_NE(entt::any_cast<int>(&any), &value);

    value = 0;

    ASSERT_EQ(entt::any_cast<int &>(any), 42);
}

TEST_F(Any, VoidAsDyn) {
    entt::any any{entt::in_place_type_dyn<void>};

    ASSERT_FALSE(any);
    ASSERT_EQ(any.type(), entt::type_info{});
}
