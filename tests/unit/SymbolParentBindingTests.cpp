#include <memory>
#include <vector>

#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "Scope.hpp"
#include "SymbolParentBinding.hpp"
#include "Symbols/All.hpp"

namespace
{
    template <typename TSymbol> auto Declare(std::unique_ptr<TSymbol> symbol) -> TSymbol*
    {
        return Ace::DiagnosticBag::CreateNoError().Collect(
            Ace::Scope::DeclareSymbol(std::move(symbol))
        );
    }

    auto Name(const char* const name) -> Ace::Ident
    {
        return Ace::Ident{ {}, name };
    }
}

auto main() -> int
{
    using namespace Ace;

    GlobalScope globalScopeOwner{ nullptr };
    const auto globalScope = globalScopeOwner.Unwrap();

    const auto functionBody = globalScope->CreateChild();
    auto* const functionTypeParam =
        Declare(std::make_unique<TypeParamTypeSymbol>(functionBody, Name("T"), 0));
    auto* const functionParam = Declare(
        std::make_unique<NormalParamVarSymbol>(functionBody, Name("value"), functionTypeParam, 0)
    );
    auto* const function = Declare(std::make_unique<FunctionSymbol>(
        functionBody,
        SymbolCategory::Static,
        AccessModifier::Pub,
        Name("identity"),
        functionTypeParam,
        std::vector<ITypeSymbol*>{}
    ));

    const auto structBody = globalScope->CreateChild();
    auto* const structTypeParam =
        Declare(std::make_unique<TypeParamTypeSymbol>(structBody, Name("T"), 0));
    auto* const structType = Declare(std::make_unique<StructTypeSymbol>(
        structBody, AccessModifier::Pub, Name("Wrapper"), std::vector<ITypeSymbol*>{}
    ));

    const auto inherentImplBody = globalScope->CreateChild();
    auto* const inherentImplTypeParam =
        Declare(std::make_unique<TypeParamTypeSymbol>(inherentImplBody, Name("T"), 0));
    auto* const inherentImpl =
        Declare(std::make_unique<InherentImplSymbol>(SrcLocation{}, inherentImplBody, structType));

    const auto traitBody = globalScope->CreateChild();
    const auto traitPrototypeScope = traitBody->CreateChild();
    auto* const trait = Declare(std::make_unique<TraitTypeSymbol>(
        traitBody,
        traitPrototypeScope,
        AccessModifier::Pub,
        Name("Container"),
        std::vector<ITypeSymbol*>{}
    ));

    const auto prototypeBody = traitPrototypeScope->CreateChild();
    auto* const prototypeTypeParam =
        Declare(std::make_unique<TypeParamTypeSymbol>(prototypeBody, Name("T"), 0));
    auto* const selfParam =
        Declare(std::make_unique<SelfParamVarSymbol>(SrcLocation{}, prototypeBody, structType));
    auto ownedPrototype = std::make_unique<PrototypeSymbol>(
        prototypeBody,
        SymbolCategory::Instance,
        Name("get"),
        0,
        trait,
        structType,
        structType,
        std::vector<ITypeSymbol*>{}
    );
    auto* const prototype = ownedPrototype.get();

    const auto traitImplBody = globalScope->CreateChild();
    auto* const traitImplTypeParam =
        Declare(std::make_unique<TypeParamTypeSymbol>(traitImplBody, Name("T"), 0));
    auto* const traitImpl =
        Declare(std::make_unique<TraitImplSymbol>(SrcLocation{}, traitImplBody, trait, structType));

    BindSymbolParents(globalScope);
    BindSymbolParents(prototype);

    ACE_ASSERT(functionTypeParam->GetParent() == function);
    ACE_ASSERT(functionParam->GetParent() == function);
    ACE_ASSERT(structTypeParam->GetParent() == structType);
    ACE_ASSERT(prototypeTypeParam->GetParent() == prototype);
    ACE_ASSERT(selfParam->GetParent() == prototype);
    ACE_ASSERT(inherentImplTypeParam->GetParent() == inherentImpl);
    ACE_ASSERT(traitImplTypeParam->GetParent() == traitImpl);
}
