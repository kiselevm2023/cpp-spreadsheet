#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

FormulaError::FormulaError(Category category) 
    : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref:
            return "#REF!";
        case Category::Value:
            return "#VALUE!";
        case Category::Arithmetic:
            return "#ARITHM!";
        }
    return "";
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        try : ast_(ParseFormulaAST(expression)) {}
        catch (const std::exception& e) {
            std::throw_with_nested(FormulaException(e.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        const std::function<double(Position)> args = 
            [&sheet](const Position p) -> double {
            if (!p.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }

            const CellInterface* cell = sheet.GetCell(p);
            if (!cell) return 0.0;
            const auto cell_value = cell->GetValue();

            return std::visit(overloaded {
                [](double val) {
                    return val;
                },
                [](const std::string& str_val) {
                    if (str_val.empty()) {
                        return 0.0;
                    }
                    double result = 0.0;
                    std::istringstream iss(str_val);
                    if (!(iss >> result) || !iss.eof()) {
                        throw FormulaError{FormulaError::Category::Value};
                    }
                    return result;
                },
                [](const FormulaError& error) -> double {
                    throw error;
                }
            }, cell_value);
        };

        try {
            return ast_.Execute(args);
        }
        catch (FormulaError& e) {
            return e;
        }
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        for (auto cell : ast_.GetCells()) {
            if (cell.IsValid()) cells.push_back(cell);
        }
        cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
        return cells;
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    } catch (...) {
        throw FormulaException("");
    }
}