module Main where

import qualified MuPDF (version)

main :: IO ()
main = do
  mupdfVersion <- MuPDF.version
  putStrLn ("mupdf: " ++ mupdfVersion)
