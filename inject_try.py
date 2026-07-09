import re

with open(r'D:\App Creation\QASM\src\main.cpp', 'r') as f:
    text = f.read()

# I will replace the main generation block with detailed try-catch blocks
text = text.replace(
    'raw_script = cores->generate_text("deep_brain", proposer_prompt);',
    'try { raw_script = cores->generate_text("deep_brain", proposer_prompt); } catch (std::exception& e) { std::ofstream err("thread_crash.log", std::ios::app); err << "CRASH deep_brain: " << e.what() << "\\n"; throw; }'
)

text = text.replace(
    'std::string review = cores->generate_text("fast_brain", reviewer_prompt);',
    'std::string review = ""; try { review = cores->generate_text("fast_brain", reviewer_prompt); } catch (std::exception& e) { std::ofstream err("thread_crash.log", std::ios::app); err << "CRASH fast_brain: " << e.what() << "\\n"; throw; }'
)

text = text.replace(
    'vm.execute_script(clean_script);',
    'try { vm.execute_script(clean_script); } catch (std::exception& e) { std::ofstream err("thread_crash.log", std::ios::app); err << "CRASH execute_script: " << e.what() << "\\n"; throw; }'
)

text = text.replace(
    'std::string ramble = cores->generate_text("deep_brain", chat_prompt);',
    'std::string ramble = ""; try { ramble = cores->generate_text("deep_brain", chat_prompt); } catch (std::exception& e) { std::ofstream err("thread_crash.log", std::ios::app); err << "CRASH voice: " << e.what() << "\\n"; throw; }'
)

with open(r'D:\App Creation\QASM\src\main.cpp', 'w') as f:
    f.write(text)
