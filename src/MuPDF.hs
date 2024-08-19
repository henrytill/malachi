module MuPDF (version) where

import Foreign.C.String (CString, peekCString)

foreign import ccall unsafe "version" c_version :: IO CString

version :: IO String
version = c_version >>= peekCString
