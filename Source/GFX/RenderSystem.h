#pragma once

#include "Core\Events.h"

class RenderSystemBase {
public:
	virtual ~RenderSystemBase() = default;

	virtual void Update(UpdateEvent& event) = 0;
	virtual void Tick(TickEvent& event) = 0;

	virtual void Render(RenderEvent& event) = 0;

	virtual float GetWeight(std::string pass, uint32_t subpass) const = 0;
	virtual float UpdateWeight() const = 0;
	virtual float TickWeight() const = 0;
};

template<typename R>
class RenderSystem : public RenderSystemBase {
public:
	virtual ~RenderSystem() = default;

	using HashKey = std::pair<std::string, uint32_t>;
	using RenderFun = std::function<void(R*, RenderEvent&)>;

	virtual void Update(UpdateEvent& event) { }
	virtual void Tick(TickEvent& event) { }

	void Render(RenderEvent& event) override {
		HashKey key(event.passName, event.subpass);
		if (renderHandlers.contains(key)) {
			renderHandlers[key].second(event);
		}
	}

	float GetWeight(std::string pass, uint32_t subpass) const override {
		HashKey key(pass, subpass);
		auto iter = renderHandlers.find(key);
		if (iter != renderHandlers.end())
			return iter->second.first;
		return 0.f;
	}

	float UpdateWeight() const override { return updateWeight; }
	float TickWeight() const override { return tickWeight; }

protected:
	RenderSystem(class Renderer& renderer, float updateWeight = 10.f, float tickWeight = 10.f)
		: renderer(renderer), system(system), updateWeight(updateWeight), tickWeight(tickWeight) {
		system = static_cast<R*>(this);
	}

	void RegisterRenderHandler(std::string pass, uint32_t subpass, float weight, RenderFun fun) {
		Assert(renderer.HasPass(pass));
		Assert(renderer.HasSubpass(pass, subpass));
		renderHandlers[HashKey(pass, subpass)] = std::pair(weight, std::bind(fun, system, std::placeholders::_1));
	}

	class Renderer& renderer;

private:
	std::unordered_map<HashKey, std::pair<float, std::function<void(RenderEvent&)>>> renderHandlers;
	float updateWeight, tickWeight;
	R* system;
};