# AI Agent Context: Construction Investment Simulation

This file provides the necessary domain knowledge, rule validation guardrails, and architectural guidelines for AI tools interacting with the project.

## 1. Project Overview & Rules
This is a turn-based economic simulation game where 2 ≤ N ≤ 5 players manage investment funds over 6 ≤ M ≤ 24 months. The game starts in **March**. The goal is to maximize total capital (K) by building houses and supermarkets, and selling apartments via a single Realtor Agency.

### Core Data & Formula Inclusions:
- **Starting Capital (K₀):** 37 million c.u. per player.
- **Objects Properties:**
  - *Panel House:* Cost 8M, Term 7 months, 60 apts, 50 sq. m.
  - *Monolith House:* Cost 10M, Term 9 months, 60 apts, 55 sq. m.
  - *Brick House:* Cost 12M, Term 10 months, 60 apts, 60 sq. m.
  - *Supermarket:* Cost 2.5M, Term 5 months.
- **Monthly Construction Cost:** \(C_{\text{month}} = \frac{C}{T}\) (Uniformly deducted at Turn Step 1).
- **Bankruptcy / Freeze:** If a player lacks cash for an active build, the build is automatically **frozen** for 1 month (no deduction, deadline shifts).

## 2. Market Mechanics & Calculations
### Real Estate Demand
- **Base Demand:** 25 apartments per month per house type.
- **Seasonality (Demand Coefficients):**
  - Jan-Feb: 0.6 | Mar: 0.7 | Apr: 0.8 | May: 0.9 | Jun: 1.0 | Jul: 1.1 | Aug: 1.2 | Sep: 1.4 | Oct: 1.5 | Nov: 1.3 | Dec: 0.8.
- **Synergy Bonus:** Each active supermarket in the microdistrict adds +4% to total demand; each under construction adds +2%.
  \[D(t) = D_{\text{base}}(t) \times (1 + 0.04 \cdot S(t))\]


### Supermarket Sales
- **Base Profit:** 0.08M per month.
- **Seasonality (Sales Coefficients):**
  - Jan: 1.5 | Feb: 1.3 | Mar: 1.1 | Apr: 0.9 | May-Jul: 0.8 | Aug: 0.9 | Sep: 1.0 | Oct: 1.1 | Nov: 1.2 | Dec: 1.4.
- **Synergy Bonus:** Each completed house in the microdistrict adds +2% to sales; each under construction adds +1%.
- **Supermarket Marketing:** Every 0.5k spent adds +3% to the profit of **each** completed supermarket owned by that player.

## 3. Turn Sequence (Strict Execution Order)
Every simulation step (month) must process actions in this exact sequence:
1. Deduct ongoing construction costs (\(C_{\text{month}}\)).
2. Credit housing sales revenue from the previous month.
3. Credit supermarket profits from the previous month.
4. Collect Player Decisions (simultaneous, using `PublicView`).
5. Validate decisions via `RulesValidator`.
6. Deduct marketing costs for houses and supermarkets.
7. Distribute market demand using `RealtorAgency` (sorted by lowest price; apply ad bonuses).
8. Update game state, asset values, and final cash pools.

## 4. Capital Evaluation (K)
Total Capital at any turn is calculated as:
\[K = C_{\text{construction}} + C_{\text{housing}} + C_{\text{markets}} + Cash\]
- \(C_{\text{construction}}\) = actual cash spent on unfinished builds.
- \(C_{\text{housing}}\) = remaining unsold apartments multiplied by their baseline production cost per sq. m.
- \(C_{\text{markets}}\) = completed supermarkets valued at \(1.6 \times C_{\text{spent}}\).
- **Final Month Rule:** Month M housing revenue must be directly credited to Cash during final settlement to prevent inventory loss.

## 5. Instructions for AI Code Generation (Token Optimization)
1. **No Complete File Refactoring:** When modifying algorithms within `GameEngine`, `RulesValidator`, or `RealtorAgency`, output **only** the impacted methods or code blocks.
2. **Context Separation:** Keep financial mathematical algorithms (`math-model`) fully decoupled from UI rendering cycles and I/O tasks.
3. **Strict Validation Mocking:** Ensure all test mocks for `RulesValidator` reject prices higher than the price ceiling (\(p_{\text{max}} = 1.5 \times 1.4 \times c\)) or configurations starting new builds without safe liquidity for the next month's first payment.

