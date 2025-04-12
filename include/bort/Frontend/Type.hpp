#pragma once
#include <boost/functional/hash.hpp>
#include <cstdint>
#include <frozen/unordered_set.h>
#include <memory>
#include <unordered_map>
#include <utility>

namespace bort {

enum class TypeKind {
  Void,
  Int,
  Char,
  Pointer,
  Array,
  NUM_TYPES
};

class Type {
public:
  virtual ~Type() = default;

  [[nodiscard]] auto getKind() const -> TypeKind {
    return m_Kind;
  };
  [[nodiscard]] auto getSizeof() const -> std::size_t {
    return m_Sizeof;
  };

  [[nodiscard]] auto is(TypeKind kind) const -> bool {
    return m_Kind == kind;
  }
  [[nodiscard]] constexpr auto isOneOf(TypeKind tk1,
                                       TypeKind tk2) const -> bool {
    return m_Kind == tk1 || m_Kind == tk2;
  }
  template <typename... TKs>
  [[nodiscard]] constexpr auto isOneOf(TypeKind tk,
                                       TKs... other) const -> bool {
    return is(tk) || isOneOf(other...);
  }

  void dump() const;
  [[nodiscard]] virtual auto toString() const -> std::string = 0;

protected:
  Type(TypeKind kind, std::size_t size)
      : m_Kind{ kind },
        m_Sizeof{ size } {
  }

private:
  TypeKind m_Kind;
  std::size_t m_Sizeof;
};

using TypeRef = std::shared_ptr<Type>;

struct TypeRefHasher {
  auto operator()(const ::bort::TypeRef& type) const noexcept
      -> std::size_t {
    std::size_t seed{ 0 };
    boost::hash_combine(seed, type->getKind());
    boost::hash_combine(seed, type->getSizeof());
    return seed;
  }
};

class VoidType final : public Type {
public:
  static auto get() -> std::shared_ptr<VoidType> {
    static std::shared_ptr<VoidType> instance{ new VoidType() };
    return instance;
  }

  [[nodiscard]] auto toString() const -> std::string override;

private:
  VoidType()
      : Type{ TypeKind::Void, 0 } {
  }
};

class IntType final : public Type {
public:
  static auto get() -> std::shared_ptr<IntType> {
    static std::shared_ptr<IntType> instance{ new IntType() };
    return instance;
  }

  [[nodiscard]] auto toString() const -> std::string override;

private:
  IntType()
      : Type{ TypeKind::Int, 4 } {
  }
};

class CharType final : public Type {
public:
  static auto get() -> std::shared_ptr<CharType> {
    static std::shared_ptr<CharType> instance{ new CharType() };
    return instance;
  }

  [[nodiscard]] auto toString() const -> std::string override;

private:
  CharType()
      : Type{ TypeKind::Char, 1 } {
  }
};

class PointerType final : public Type {
  using SignatureT = TypeRef;

public:
  [[nodiscard]] auto getPointee() const -> TypeRef {
    return m_Pointee;
  }

  [[nodiscard]] auto toString() const -> std::string override;

  static auto get(const TypeRef& type) -> std::shared_ptr<PointerType> {
    static std::unordered_map<SignatureT, std::shared_ptr<PointerType>>
        instances;
    const SignatureT& signature{ type }; // just to follow general trend
    if (!instances.contains(signature)) {
      instances[signature] =
          std::shared_ptr<PointerType>(new PointerType(type));
    }
    return instances.at(signature);
  }

private:
  explicit PointerType(TypeRef type)
      : Type{ TypeKind::Pointer, 4 },
        m_Pointee{ std::move(type) } {
  }

  TypeRef m_Pointee;
};

class ArrayType final : public Type {
  using SignatureT = std::pair<TypeRef, std::size_t>;

public:
  [[nodiscard]] auto getBaseType() const -> TypeRef {
    return m_BaseType;
  }

  [[nodiscard]] auto getNumElements() const -> uint32_t {
    return m_NumElements;
  }

  [[nodiscard]] auto toString() const -> std::string override;

  static auto get(const TypeRef& type,
                  std::size_t numElements) -> std::shared_ptr<ArrayType> {

    struct ArraySignatureHasher {
      auto operator()(const std::pair<TypeRef, std::size_t>& pair) const
          -> std::size_t {
        std::size_t seed = 0;
        boost::hash_combine(seed, pair.first);
        boost::hash_combine(seed, pair.second);
        return seed;
      }
    };

    static std::unordered_map<std::pair<TypeRef, std::size_t>,
                              std::shared_ptr<ArrayType>,
                              ArraySignatureHasher>
        instances;
    SignatureT signature{ type, numElements };
    if (!instances.contains(signature)) {
      instances[signature] =
          std::shared_ptr<ArrayType>(new ArrayType(type, numElements));
    }
    return instances.at(signature);
  }

private:
  ArrayType(TypeRef type, std::size_t numElements)
      : Type{ TypeKind::Array, type->getSizeof() * numElements },
        m_BaseType{ std::move(type) },
        m_NumElements{ numElements } {
  }

  TypeRef m_BaseType;
  std::size_t m_NumElements;
};

} // namespace bort
