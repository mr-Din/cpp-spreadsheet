#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <functional>
#include <unordered_set>

// Тип ячейки
enum class CellType
{
    EMPTY,    // default type on cell creation
    TEXT,
    FORMULA,
    ERROR
};

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(const std::string& text);
    void Clear();

    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsCyclicDependent(const Cell* start_cell_ptr, const Position& end_pos) const;
    void InvalidateCache();
    bool IsCacheValid() const;

private:
    class Impl;    // Forward declaration

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;

    class Impl {
    public:
        virtual ~Impl() = default;
        virtual CellType IGetType() const = 0;
        virtual CellInterface::Value IGetValue() const = 0;
        virtual std::string IGetText() const = 0;

        virtual std::vector<Position> IGetReferencedCells() const = 0;
        virtual void IInvalidateCache() = 0;
        virtual bool ICached() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        CellType IGetType() const override;
        CellInterface::Value IGetValue() const override;
        std::string IGetText() const override;

        std::vector<Position> IGetReferencedCells() const override; 
        void IInvalidateCache() override;
        bool ICached() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);
        CellType IGetType() const override;
        CellInterface::Value IGetValue() const override;
        std::string IGetText() const override;  

        std::vector<Position> IGetReferencedCells() const override;
        void IInvalidateCache() override;
        bool ICached() const override;
    private:
        std::string cell_text_;
        bool escaped_ = false;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(SheetInterface& sheet_, std::string formula);
        CellType IGetType() const override;
        CellInterface::Value IGetValue() const override; 
        std::string IGetText() const override;  

        std::vector<Position> IGetReferencedCells() const override;
        void IInvalidateCache() override;
        bool ICached() const override;
    private:
        SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
        std::optional<CellInterface::Value> cached_value_;
    };
};