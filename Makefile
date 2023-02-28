SHADER_DIR = Shaders
COMPILER = "C:\Dev\VulkanSDK\Bin\glslc.exe"

VERT_TARGETS = $(addsuffix .spv,$(wildcard $(SHADER_DIR)/*.vert))
FRAG_TARGETS = $(addsuffix .spv,$(wildcard $(SHADER_DIR)/*.frag))
COMP_TARGETS = $(addsuffix .spv,$(wildcard $(SHADER_DIR)/*.comp))

.PHONY: all

all: $(VERT_TARGETS) $(FRAG_TARGETS) $(COMP_TARGETS)
	@:

%.vert.spv: %.vert
	@echo $(notdir $<)$
	@$(COMPILER) $< -o $@

%.frag.spv: %.frag
	@echo $(notdir $<)$
	@$(COMPILER) $< -o $@

%.comp.spv: %.comp
	@echo $(notdir $<)$
	@$(COMPILER) $< -o $@