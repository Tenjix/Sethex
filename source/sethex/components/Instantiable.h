#pragma once

#include <sethex/Common.h>
#include <sethex/EntitySystem.h>
#include <sethex/Graphics.h>

namespace sethex {

	class Instantiable : public ObservableComponent {

	public:

		SharedProperty<Batch, Instantiable> batch;

		Instantiable(const shared<Batch>& batch = nullptr) : batch(batch) {
			this->batch.owner = this;
			this->batch.attach([this]() { notify(); });
		}

		static shared<Instantiable> create(const shared<Batch>& batch = nullptr) {
			return std::make_shared<Instantiable>(batch);
		}

		void instantiate(uint number_of_instances = 1) {
			runtime_assert(batch, "instantiable requires a batch object");
			runtime_assert(number_of_instances < INT_MAX, "number of instances may not exceed 32 bit integer range");
			batch->drawInstanced(static_cast<unsigned>(number_of_instances));
		}

	};

}
