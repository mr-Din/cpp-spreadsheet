#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <cmath>

using namespace std::literals;

Cell::Cell(SheetInterface& sheet)
    : sheet_(sheet)
{}

Cell::~Cell()
{
    if (impl_) {
        impl_.reset(nullptr);
    }
}

void Cell::Set(const std::string& text) {
    
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    }

    if (text[0] != FORMULA_SIGN || (text[0] == FORMULA_SIGN && text.size() == 1)) {
        impl_ = std::make_unique<TextImpl>(text);
        return;
    }

    try {
        impl_ = std::make_unique<FormulaImpl>(sheet_, std::string{ text.begin() + 1, text.end() });
    }
    catch (...) {
        std::string fe_msg = "Formula parsing error"s;
        throw FormulaException(fe_msg);
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->IGetValue();
}

std::string Cell::GetText() const {
    return impl_->IGetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_.get()->IGetReferencedCells();
}

bool Cell::IsCyclicDependent(const Cell* start_cell_ptr, const Position& end_pos) const {
    
    // Проверка зависимых ячеек
    for (const auto& referenced_cell_pos : GetReferencedCells()) {

        if (referenced_cell_pos == end_pos) {
            return true;
        }

        const Cell* ref_cell_ptr = dynamic_cast<const Cell*>(sheet_.GetCell(referenced_cell_pos));

        if (!ref_cell_ptr) {
            sheet_.SetCell(referenced_cell_pos, "");
            ref_cell_ptr = dynamic_cast<const Cell*>(sheet_.GetCell(referenced_cell_pos));
        }

        if (start_cell_ptr == ref_cell_ptr) {
            return true;
        }

        if (ref_cell_ptr->IsCyclicDependent(start_cell_ptr, end_pos)) {
            return true;
        }
    }

    return false;
}

void Cell::InvalidateCache() {
    impl_->IInvalidateCache();
}

bool Cell::IsCacheValid() const {
    return impl_->ICached();
}


CellType Cell::EmptyImpl::IGetType() const {
    return CellType::EMPTY;
}

CellInterface::Value Cell::EmptyImpl::IGetValue() const {
    return 0.0;
}

std::string Cell::EmptyImpl::IGetText() const {
    return ""s;
}

std::vector<Position> Cell::EmptyImpl::IGetReferencedCells() const {
    return {};
}

void Cell::EmptyImpl::IInvalidateCache() {
    return;
}

bool Cell::EmptyImpl::ICached() const {
    return true;
}

/*---------------------------------------------------------------*/

Cell::TextImpl::TextImpl(std::string text)
    : cell_text_(std::move(text))
{

    if (cell_text_[0] == ESCAPE_SIGN) {
        escaped_ = true;
    }
}

CellType Cell::TextImpl::IGetType() const {
    return CellType::TEXT;
}

CellInterface::Value Cell::TextImpl::IGetValue() const {
    if (escaped_) {
        return cell_text_.substr(1, cell_text_.size() - 1);
    }
    else {
        return cell_text_;
    }
}

std::string Cell::TextImpl::IGetText() const {
    return cell_text_;
}

std::vector<Position> Cell::TextImpl::IGetReferencedCells() const {
    return {};
}

void Cell::TextImpl::IInvalidateCache() {
    return;
}

bool Cell::TextImpl::ICached() const {
    return true;
}

/*---------------------------------------------------------------*/

Cell::FormulaImpl::FormulaImpl(SheetInterface& sheet, std::string formula)
    : sheet_(sheet), formula_(ParseFormula(formula))
{}

CellType Cell::FormulaImpl::IGetType() const {
    return CellType::FORMULA;
}

CellInterface::Value Cell::FormulaImpl::IGetValue() const {
    FormulaInterface::Value result = formula_->Evaluate(sheet_);
    if (std::holds_alternative<double>(result)) {

        if (std::isfinite(std::get<double>(result))) {
            return std::get<double>(result);
        }
        else {
            return FormulaError(FormulaError::Category::Div0);
        }
    }

    return std::get<FormulaError>(result);
}

std::string Cell::FormulaImpl::IGetText() const {
    return { FORMULA_SIGN + formula_->GetExpression() };
}

std::vector<Position> Cell::FormulaImpl::IGetReferencedCells() const {
    return formula_.get()->GetReferencedCells();
}

void Cell::FormulaImpl::IInvalidateCache() {
    cached_value_.reset();
}

bool Cell::FormulaImpl::ICached() const {
    return cached_value_.has_value();
}
