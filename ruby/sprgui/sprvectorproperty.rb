include Fox

class SprVectorProperty < FXHorizontalFrame
	def initialize(owner, ndim)
		super(owner, FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y)
		@ndim = ndim
		@vector = Array(@ndim)
		@vector.each { |elem|
			elem = FXTextField.new(self, 5, nil, 0, (TEXTFIELD_REAL|JUSTIFY_RIGHT|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW))
		}
	end

	def update(v, upload)
		for i in 0..(@ndim-1)
			if upload
				@vector[i].text = v[i].to_s
			else
				v[i] = @vector[i].text.to_f
			end
		end
	end
end