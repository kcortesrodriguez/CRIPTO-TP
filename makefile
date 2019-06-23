default:
	@echo "Building project"
	@echo "-"
	@rm -rf build
	@echo "\\"
	@mkdir build
	@cd build && cmake .. >/dev/null && echo "|" >/dev/null && make >/dev/null
	@echo "/"
	@echo "Cleaning"
	@echo "-"
	@mv build/CRIPTO_TP CRIPTO_TP
	@echo "\\"
	@rm -rf build
	@echo "Cheers!!! Happy crypting!"
