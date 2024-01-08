#pragma once

#include <array>
#include <optional>
#include <variant>

constexpr int PARTY_COLS = 3;
constexpr int PARTY_ROWS = 2;
constexpr int PARTY_SIZE = PARTY_COLS * PARTY_ROWS;

struct SBaseChar
{
    char Name[12]{};
    float Health{};
    float MaxHealth{};
    int Initiative{};

    int Size{};
    bool bHorizontal{};
};

struct SMonster : SBaseChar
{
};

struct SPlayerChar : SBaseChar
{
};

struct SPartySlot
{
private:
    std::variant<std::monostate, SBaseChar, SBaseChar*> Slot;

public:
    [[nodiscard]] bool IsEmpty() const
    {
        return std::holds_alternative<std::monostate>(Slot);
    }

    [[nodiscard]] bool IsRealChar() const
    {
        return std::holds_alternative<SBaseChar>(Slot);
    }

    [[nodiscard]] bool IsSecondaryChar() const
    {
        return std::holds_alternative<SBaseChar*>(Slot);
    }

    void SetEmpty()
    {
        Slot = std::monostate();
    }

    void SetRealChar(const SBaseChar& Char)
    {
        Slot = Char;
    }

    void SetSecondaryChar(SBaseChar* Char)
    {
        Slot = Char;
    }

    SBaseChar& GetRealChar()
    {
        return std::get<SBaseChar>(Slot);
    }

    SBaseChar* GetSecondaryChar()
    {
        return std::get<SBaseChar*>(Slot);
    }

    SBaseChar* GetRealCharPtr()
    {
        return &std::get<SBaseChar>(Slot);
    }
};

struct SParty
{
    std::array<SPartySlot, PARTY_SIZE> Slots{};

    bool AddCharacter(const SBaseChar& Char)
    {
        int I = 0;
        for (; I < PARTY_SIZE; ++I)
        {
            if (!Slots[I].IsEmpty())
                continue;
            if (Char.Size == 1)
            {
                Slots[I].SetRealChar(Char);
                return true;
            }
            if (Char.Size == 2)
            {
                int NextSlotIndex = Char.bHorizontal ? I + 1 : I + (PARTY_SIZE / 2);
                bool bNextSlotIsFree = NextSlotIndex < PARTY_SIZE && Slots[NextSlotIndex].IsEmpty();
                if (bNextSlotIsFree)
                {
                    Slots[I].SetRealChar(Char);
                    Slots[NextSlotIndex].SetSecondaryChar(Slots[I].GetRealCharPtr());
                    return true;
                }
            }
        }
        return false;
    }

    [[nodiscard]] int GetPartyCount() const
    {
        auto Count = 0;
        for (const auto& Slot : Slots)
        {
            if (Slot.IsRealChar())
                Count++;
        }
        return Count;
    }
};
