import gdb

class CommandPrinter:
	def __init__(self, val):
		self.val = val

	def to_string(self):
		# Extract raw values
		dest = int(self.val['m_dest'])
		vala = int(self.val['m_vala'])
		valb = int(self.val['m_valb'])
		
		# Map OperationType_impl to string
		# We convert to int because 'm_operation' is an enum/u8
		op_value = int(self.val['m_operation'])
		op_map = {
			1: "Add",
			2: "Subtract",
			3: "Multiply",
			4: "Divide",
			5: "Assign"
		}
		op_name = op_map.get(op_value, f"Unknown({op_value})")

		# Helper to decode your 2-bit flags
		def decode_val(v):
			# Binary representation (8-bit)
			bin_str = format(v, '08b')
			# Extract bits
			is_from_buffer = (v & 0x80) != 0
			is_var_buffer = (v & 0x40) != 0
			value = v & 0b00111111
			
			if is_from_buffer:
				buf_type = "Var" if is_var_buffer else "Const"
				return f"{bin_str} (Buffer/{buf_type}:{value})"
			else:
				return f"{bin_str} (Reg:{value})"

		return f"Dest: {dest:<2} | Op: {op_name:12} | valA: {decode_val(vala):28} | valB: {decode_val(valb)}"

def lookup_tx_type(val):
	# Use 'Command' or 'tx::Command' depending on how it's defined in your code
	type_str = str(val.type).strip()
	if "Command" in type_str:
		return CommandPrinter(val)
	return None

# Register the printer
gdb.pretty_printers.append(lookup_tx_type)
print("TXLib Debug Printers Loaded.")