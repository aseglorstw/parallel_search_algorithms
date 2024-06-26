#pragma once

#include <iostream>
#include <bit>
#include <sstream>
#include "../state.h"

namespace hanoi {
    constexpr unsigned int LOG2worker(unsigned int x) {
        return (x == 1) ? 0 : 1 + LOG2worker(x >> 1);
    }

    constexpr unsigned int LOG2(unsigned int x) {
        return (x & (x - 1)) ? 1 + LOG2worker(x) : LOG2worker(x);
    }

    template<unsigned int RODS, unsigned int TOWERS, unsigned int DISCS>
    class hanoi_state : public state, public std::enable_shared_from_this<hanoi_state<RODS, TOWERS, DISCS>> {
    private:
        const std::vector<unsigned int> conf;
        unsigned long long id;

    public:
        hanoi_state(const state_ptr predecessor, unsigned int cost, const std::vector<unsigned int>& conf)
            : state(predecessor, cost), conf(conf) {
            id = 0ull;
            for (unsigned int disc = 0; disc < DISCS; ++disc) {
                unsigned int mask = 1u << disc;
                unsigned int rod = 0;
                for (unsigned int tower = 0; tower < TOWERS; ++tower) {
                    for (; !(conf[rod] & mask); ++rod);
                    id = (id << LOG2(RODS)) | rod;
                }
            }
        }

        ~hanoi_state() {}

        std::vector<state_ptr> next_states() const override {
            auto tmp_conf = conf;
            std::vector<state_ptr> succ;
            for (unsigned int s = 0; s < RODS; ++s) {
                auto sconf = conf[s];
                if (!sconf) continue;

                unsigned int sdisk_ind = (1 << std::countr_zero(sconf));
                unsigned int sdisk_mask = (sdisk_ind << 1) - 1;

                for (unsigned int t = 0; t < RODS; ++t) {
                    if (conf[t] & sdisk_mask) continue;
                    else {
                        tmp_conf[s] ^= sdisk_ind;
                        tmp_conf[t] ^= sdisk_ind;

                        succ.push_back(
                            std::make_shared<const hanoi_state<RODS, TOWERS, DISCS>>(this->shared_from_this(),
                                                                                     current_cost() + 1, tmp_conf));

                        tmp_conf[s] ^= sdisk_ind;
                        tmp_conf[t] ^= sdisk_ind;
                    }
                }
            }
            return succ;
        }

        bool is_goal() const override {
            unsigned int mask = (1 << DISCS) - 1;
            unsigned int crod = RODS - 1;
            for (unsigned int i = 0; i < TOWERS; i++) {
                if (conf[crod--] != mask) return false;
            }
            return true;
        }

        unsigned long long int get_identifier() const override {
            return id;
        }

        std::string to_string() const override {
            std::ostringstream out;
            for (unsigned int i = 0; i < RODS; i++) {
                unsigned int rconf = conf[i];
                while (rconf) {
                    unsigned int disc = std::countr_zero(rconf);
                    rconf ^= (1 << disc);
                    out << disc << " ";
                }
                out << "| ";
            }
            return out.str();
        }
    };

    template<unsigned int RODS, unsigned int TOWERS, unsigned int DISCS>
    class domain {
    public:
        state_ptr get_root() {

            std::cout << "Domena Hanojske veze\n";
            std::cout << "Pocet koliku = " << RODS << ", vyska veze = " << DISCS << ", pocet vezi = " << TOWERS
                      << "\n";
            std::cout << "\n";

            std::vector<unsigned int> conf(RODS);
            for (unsigned int i = 0; i < TOWERS; i++) {
                conf[i] = (1 << DISCS) - 1;
            }

            return std::make_shared<hanoi_state<RODS, TOWERS, DISCS>>(std::shared_ptr<state>(), 0, conf);
        }

    };

    // explicit instantiation to get type checking
    template
    class domain<2, 1, 1>;
}
